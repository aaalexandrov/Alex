import ODBC
import YAML

const dbName = "esb_DATADUMP"

function import_decryptors(fileName::AbstractString)
	ODBC.connect(dbName)
	res = ODBC.query(
		"""
		select ta.typeID, typeName, attributeName, valueFloat
		from dbo.dgmTypeAttributes ta, dbo.dgmAttributeTypes at, dbo.invTypes it
		where ta.typeID in (select typeID 
						    from dbo.invTypes it, dbo.invMarketGroups mg
						    where it.marketGroupID=mg.marketGroupID 
							      and mg.marketGroupName='Decryptors') 
			  and ta.attributeID=at.attributeID
			  and ta.typeID = it.typeID
		order by typeID, ta.attributeID
		"""
	)
	ODBC.disconnect()
	# ODBC.DataFrames.writetable("data/Decryptors.csv", res)
	decID = -1
	decryptors = {}
	for row = 1:size(res, 1)
		if decID != res[row, :typeID]
			decID = res[row, :typeID]
			dec = Dict{AbstractString, Any}()
			dec["id"] = decID
			dec["name"] = res[row, :typeName]
			dec["attributes"] = Dict{AbstractString, Float64}()
			push!(decryptors, dec)
		end
		dec["attributes"][res[row, :attributeName]] = res[row, :valueFloat]
	end
	json_write(fileName, decryptors)
end

function import_blueprints(fileName::AbstractString)
	# because loading a json is so much faster than loading a yaml for some reason
	blueprints = YAML.load_file("data/blueprints.yaml")
	json_write(fileName, blueprints; indent=0)
end

function import_station_assembly_lines(fileName::AbstractString)
	ODBC.connect(dbName)
	res = ODBC.query(
		"""
		select als.stationID, stationName, s.solarSystemID, constellationID, s.regionID, baseTimeMultiplier, baseMaterialMultiplier, baseCostMultiplier, minCostPerHour, ast.activityID, activityName 
		from dbo.ramAssemblyLineStations als, dbo.ramAssemblyLineTypes ast, dbo.staStations s, dbo.ramActivities a
		where als.assemblyLineTypeID = ast.assemblyLineTypeID and als.stationID = s.stationID and ast.activityID = a.activityID	
		"""
	)
	#ODBC.DataFrames.writetable("data/AssemblyLines.csv", res)
	ODBC.disconnect()
	json_write(fileName, make_array(res))
end