using AstroLib, LinearAlgebra, CSV, DataFrames, Dates, LinearRegression

const dateDefault = "2024-08-01T12:00:00.000" # UTC?

struct Location
    lattitude::Float64
    longitude::Float64
    altitude::Float64
end

const Karaisen = Location(43.373243, 25.351685, 105.3)
const PanelAltAzi = (90 - 25, 0)
const PanelWpeak = 450
const PanelNum = 70
const InverterEfficiency = 0.95

function SunPosLocal(time, loc::Location; ws = true)
    jd = jdcnv(time)
    ra, dec = sunpos(jd)
    alt, azi = eq2hor(ra, dec, jd, loc.lattitude, loc.longitude, loc.altitude; ws = ws)
    alt, azi
end

function AltAzi2Vec(alt, azi)
    sinAlt = sin(deg2rad(alt))
    cosAlt = cos(deg2rad(alt))
    sinAzi = sin(deg2rad(azi))
    cosAzi = cos(deg2rad(azi))
    [cosAlt*cosAzi, cosAlt*sinAzi, sinAlt]
end

function SunDirectIntensity(alt)
    airMass = 1 / cos(deg2rad(90 - alt))
    if airMass <= 0
        return 0
    end
    1.353*0.7^(airMass^0.678)
end

function PanelWatts(time, loc = Karaisen)
    sunAlt, sunAzi = SunPosLocal(time, loc)

    #TODO: when the sun is below ~13 degrees altitude, a better fit seems to be linearly transitioning to 0 at sunrise / sunset time

    sunDir = AltAzi2Vec(sunAlt, sunAzi)
    panelDir = AltAzi2Vec(PanelAltAzi...)
    sunDirCoef = max(0, dot(sunDir, panelDir) * (sunAlt > 0))
    airMassCoef = SunDirectIntensity(sunAlt)
    
    PanelNum * PanelWpeak * sunDirCoef * airMassCoef * InverterEfficiency
end

TimeStrToTimeUTC(date, timeOffsetHours) = 
    DateTime(date[1:end-4], dateformat"Y-m-d H:M:S") + Dates.Hour(timeOffsetHours)

function LoadSolarData(file, timeOffsetHours = -3)
    dataRaw = DataFrame(CSV.File(file))
    rows = size(dataRaw, 1)
    data = DataFrame(
        datetime = [TimeStrToTimeUTC(d, timeOffsetHours) for d in dataRaw[:, 1]], 
        kwh = dataRaw[:, 2], 
        kwhDelta = [dataRaw[min(i+1, rows), 2] - dataRaw[i, 2] for i in 1:rows],
    )
    data[!, :timeDelta] = [data.datetime[min(i+1, rows)] - data.datetime[i] for i in 1:rows]
    data
end

function LoadWeatherData(file)
    DataFrame(CSV.File(file))
end

lerp(a, b, w) = a + (b - a) * w

function SampleWeather(weather, time)
    range = searchsorted(weather.datetime[:], time)
    if !isempty(range)
        return weather[first(range), :], range
    end
    r0 = weather[max(1, first(range) - 1), :]
    r1 = weather[min(size(weather, 1), first(range)), :]
    timeLen = r1.datetime - r0.datetime
    w = timeLen == Dates.Second(0) ? 0 : (time - r0.datetime) / timeLen
    row = DataFrame(w < 0.5 ? r0 : r1)
    colNames = names(row)
    colTypes = eltype.(eachcol(row))
    for c = 1:size(row, 2) 
        if colNames[c] == "datetime"
            row[1, c] = time
            continue
        end
        if colTypes[c] <: Number
            val = lerp(r0[c], r1[c], w)
            if !(colTypes[c] <: AbstractFloat)
                val = floor(val)
            end
            row[1, c] = convert(colTypes[c], val)
        end
    end
    row, range
end

function ResampleWeather!(weather, times)
    for t in times
        row, place = SampleWeather(weather, t)
        if isempty(place)
            insert!(weather, first(place), row[1, :])
        end
    end
end

const overcastCoef = 0.47
const clearCoef = 1.0
const calibrationCoef = 0.95

function EstimateKwhIdeal(row)
    kw = PanelWatts(row.datetime) / 1000
    kwh = kw * (row.timeDelta / Dates.Hour(1))
    kwh
end

function EstimateKwh(row)
    kwh = EstimateKwhIdeal(row)
    cloudcover = (1 - row.cloudcover / 100)
    cloudsCoef = overcastCoef + (clearCoef - overcastCoef) * cloudcover
    kwh * cloudsCoef * calibrationCoef
    
    # coefs of linear regression
    # kwh * 0.788140623984657 + row.cloudcover * -0.0046779734601801225 + 0.15382362617263193
end    

function Estimate(power, weather)
    resampledWeather = DataFrame(weather[:, :])
    ResampleWeather!(resampledWeather, power.datetime)
    estimate = innerjoin(power, resampledWeather, on = :datetime)
    estimate[!, :kwhEstimate] = [EstimateKwh(row) for row in eachrow(estimate)]
    estimate[!, :kwhIdeal] = [EstimateKwhIdeal(row) for row in eachrow(estimate)]
    estimate
end

function DoEstimate()
    power = LoadSolarData("KaraisenPower.csv")
    weather = LoadWeatherData("KaraisenWeather.csv")
    est = Estimate(power, weather)
    CSV.write("KaraisenEstimate.csv", est)
    # @df est plot([:kwhDelta, :kwhEstimate])
    est
end

function LinRegressCoef(estimate)
    lr=linregress(hcat(est.kwhIdeal, est.cloudcover), est.kwhDelta)
    coef(lr)
end