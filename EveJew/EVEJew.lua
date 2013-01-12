require("luaxml")
require("socket.http")

skills = {
  ["Amarr Encryption Methods"] = { level = 4 },
  ["Amarrian Starship Engineering"] = { level = 4 },
  ["Mechanical Engineering"] = { level = 4 },

  ["Minmatar Encryption Methods"] = { level = 4 },
  ["Minmatar Starship Engineering"] = { level = 4 },

  ["Gallente Encryption Methods"] = { level = 4 },
  ["Gallentean Starship Engineering"] = { level = 4 },

  ["Caldari Encryption Methods"] = { level = 4 },
  ["Caldari Starship Engineering"] = { level = 4 },

  ["Hydromagnetic Physics"] = { level = 3 },
  ["Quantum Physics"] = { level = 3 },

  ["High Energy Physics"] = { level = 3 },
  ["Laser Physics"] = { level = 3 },

  ["Electronic Engineering"] = { level = 3 },
  ["Nuclear Physics"] = { level = 3 },
}

default_me = 10
default_skill = 3

materials = {
  ["Tempest Fleet Issue"] = { me = 0 },
  ["Armageddon"] = { me = 27 },
  ["Apocalypse"] = { me = 25 },

  ["R.A.M.- Starship Tech"] =	{ me = 30 },

  ["Standard Drop Booster"] = { me = 0 },
  ["Improved Drop Booster"] = { me = 0 },
  ["Strong Drop Booster"] = { me = 0 },

  ["Standard Blue Pill Booster"] = { me = 0 },
  ["Improved Blue Pill Booster"] = { me = 0 },
  ["Strong Blue Pill Booster"] = { me = 0 },

  ["Standard Crash Booster"] = { me = 0 },
  ["Improved Crash Booster"] = { me = 0 },
  ["Strong Crash Booster"] = { me = 0 },

  ["Standard Exile Booster"] = { me = 0 },
  ["Improved Exile Booster"] = { me = 0 },
  ["Strong Exile Booster"] = { me = 0 },

  ["Standard Frentix Booster"] = { me = 0 },
  ["Improved Frentix Booster"] = { me = 0 },
  ["Strong Frentix Booster"] = { me = 0 },

  ["Standard Mindflood Booster"] = { me = 0 },
  ["Improved Mindflood Booster"] = { me = 0 },
  ["Strong Mindflood Booster"] = { me = 0 },

  ["Standard Sooth Sayer Booster"] = { me = 0 },
  ["Improved Sooth Sayer Booster"] = { me = 0 },
  ["Strong Sooth Sayer Booster"] = { me = 0 },

  ["Standard X-Instinct Booster"] = { me = 0 },
  ["Improved X-Instinct Booster"] = { me = 0 },
  ["Strong X-Instinct Booster"] = { me = 0 },
}

function round(n)
  return math.floor(n + 0.5)
end

function FileExists(filename)
  local f
  f = io.open(filename, "r")
  if f ~= nil then
    f:close()
    return true
  end
  return false
end

function LoadTypeIDs()
  local f, second
  typeID = {}
  f = io.open("TypesMarket.csv")
  for line in f:lines() do
    if second then
      local t, g, m, n
      t, g, m, n = line:match("(.+);(.+);(.+);(.+)")
      typeID[n] = { typeID = t, groupID = g, marketID = m }
    else
      second = true
    end
  end
  f:close()
end

function LoadInventionData()
  local f, second
  invention = {}
  f = io.open("InventionMaterials.csv")
  for line in f:lines() do
    if second then
      local n, m, q, s, d, row
      n, m, q, s, d = line:match("(.+);(.+);(.+);(.+);(.+)")
      if d == 'NULL' then
        d = nil
      end
      if not invention[n] then
        invention[n] = {}
      end
      row = { material = m, quantity = q, skill = s, decryptors = d }
      if d then
        invention[n].encryption = row
      else
        if not invention[n].scientific1 then
          invention[n].scientific1 = row
        else
          invention[n].scientific2 = row
        end
      end
    else
      second = true
    end
  end
  f:close()
end

function LoadBlueprints()
  local f, second
  f = io.open("Blueprints.csv")
  for line in f:lines() do
    if second then
      local p, w, r, t, b
      p, w, r, t, b = line:match("(.+);(.+);(.+);(.+);(.+)")
      if not materials[p] then
        materials[p] = { me = default_me }
      end
      w = tonumber(w)
      r = tonumber(r)
      t = tonumber(t)
      if b == 'NULL' then
        b = nil
      end
      materials[p].basewaste = w / 100
      materials[p].maxRuns = r
      materials[p].techLevel = t
      materials[p].baseBlueprint = b
    else
      second = true
    end
  end
  f:close()
end

function LoadMaterials()
  local f, second
  f = io.open("BlueprintMaterials.csv")
  for line in f:lines() do
    if second then
      local p, m, q, d
      p, m, q, d = line:match("(.+);(.+);(.+);(.+)")
      if not materials[p] then
        materials[p] = { me = default_me }
      end
      if not materials[p].materials then
        materials[p].materials = {}
      end
      q = tonumber(q)
      d = tonumber(d)
      materials[p].materials[m] = q * d
    else
      second = true
    end
  end
  f:close()
end

function LoadShipGroups()
  local f, second
  f = io.open("ShipGroups.csv")
  for line in f:lines() do
    if second then
      local s, g
      s, g = line:match("(.+);(.+)")
      if materials[s] ~= nil then
        materials[s].shipGroup = g
      end
    else
      second = true
    end
  end
  f:close()
  ResolveBaseChance()
end

function ResolveBaseChance()
  for i, t in pairs(materials) do
    local chance, group
    chance = nil
    if t.materials then
      for mat, q in pairs(t.materials) do
        group = materials[mat] and materials[mat].shipGroup
        if group == 'Frigate' or group == 'Destroyer' or group == 'Freighter' or i == 'Skiff' then
          chance = 0.3
        elseif group == 'Cruiser' or group == 'Industrial' or i == 'Mackinaw' then
          chance = 0.25
        elseif group == 'Battlecruiser' or group == 'Battleship' or i == 'Hulk' then
          chance = 0.2
        end
      end
    end
    if not chance and invention[i] then
      chance = 0.4
    end
    if chance then
      t.basechance = chance
    end
  end
end

function LoadReactions()
  local f, second
  reactions = {}
  f = io.open("Reactions.csv")
  for line in f:lines() do
    if second then
      local r, i, m, q
      r, i, m, q = line:match("(.+);(.+);(.+);(.+)")
      if not reactions[r] then
        reactions[r] = { name = r, inputs = {}, outputs = {} }
      end
      i = tonumber(i)
      q = tonumber(q)
      if i ~= 0 then
        reactions[r].inputs[m] = q
      else
        reactions[r].outputs[m] = q
      end
    else
      second = true
    end
  end
  f:close()
  ResolveReactionProducts()
end

function ResolveReactionProducts()
  local mainProduct
  local newReactions = {}
  for name, reaction in pairs(reactions) do
    mainProduct = nil
    for product, q in pairs(reaction.outputs) do
      if reaction.inputs[product] == nil then
        if mainProduct ~= nil then
          print("Reaction " .. name .. " has multiple products")
        end
        mainProduct = product
      end
    end
    if mainProduct == nil then
      print("Reaction " .. name .. " does not have a valid product")
    else
      newReactions[mainProduct] = reaction
    end
  end
  reactions = newReactions
end

function LoadDecryptors()
  local f, second, decr
  decr = {}
  f = io.open("DecryptorAttributes.csv")
  for line in f:lines() do
    if second then
      local d, v, a, g
      d, v, a, g = line:match("(.+);(.+);(.+);(.+)")
      if not decr[d] then
        decr[d] = {}
      end
      v = tonumber(v)
      if a == 'inventionMEModifier' then
        decr[d].me = v
      elseif a == 'inventionMaxRunModifier' then
        decr[d].runs = v
      elseif a == 'inventionPEModifier' then
        decr[d].pe = v
      elseif a == 'inventionPropabilityMultiplier' then
        decr[d].probability = v
      end
      decr[d].group = g
    else
      second = true
    end
  end
  decryptors = {
    { runs = 0, probability = 1, me = 0, pe = 0 }
  }
  for d, t in pairs(decr) do
    local set, row
    set = nil
    for i, da in ipairs(decryptors) do
      if da.runs == t.runs and da.probability == t.probability and da.me == t.me and da.pe == t.pe then
        da[t.group] = d
        decryptors[1][t.group] = 'None'
        set = true
      end
    end
    if not set then
      row = { runs = t.runs, probability = t.probability, me = t.me, pe = t.pe }
      row[t.group] = d
      decryptors[#decryptors + 1] = row
      decryptors[1][t.group] = 'None'
    end
  end
  f:close()
end

function LoadPriceXML()
  prices = {}
  local price_xml
  if FileExists("price.xml") then
    price_xml = xml.load("price.xml")
    if price_xml ~= nil then
      local i
      for i = 1, #price_xml do
        local price
        price = price_xml[i]
        prices[price.name] = {}
        prices[price.name].price = tonumber(price.price)
        prices[price.name].time = price.time and tonumber(price.time) or nil
      end
    end
  end
end

function SavePriceXML()
  local price_xml = {}
  price_xml[0] = "price_xml"
  for k, v in pairs(prices) do
    if v.price > 0 then
      price_xml[#price_xml + 1] = { [0] = "item", [1] = nil, name = k, price = v.price, time = v.time }
    end
  end
  price_xml[#price_xml+1] = nil
--  price_xml.time = os.time()
  xml.save(price_xml, "price.xml")
end

function FetchPrice(k)
  local type_id = typeID[k] and typeID[k].typeID
  if type_id == nil then
    return nil
  end
  local r, t, sell, min_price
  -- request Jita prices only
  r = socket.http.request("http://api.eve-central.com/api/quicklook?usesystem=30000142&typeid=" .. tostring(type_id))
  t = xml.eval(r)
  print(k)
  assert(string.find(xml.find(t, "itemname")[1], k, 1, true))
  sell = xml.find(t, "sell_orders")
  min_price = nil
  for n, o in pairs(sell) do
    local ord, ord_price
    ord = xml.find(o, "price")
    if ord ~= nil then
      ord_price = tonumber(ord[1])
      if min_price == nil or ord_price < min_price then
        min_price = ord_price
      end
    end
  end
  if min_price == nil then
    print("Missing price for ", k)
  else
    SavePriceXML()
  end
  return min_price
end

function GetPrice(sProduct)
  local price, min_price, timeNow
  timeNow = os.time()
  price = prices[sProduct]
  if price and price.time and os.difftime(timeNow, price.time) < 24 * 60 * 60 then
    return price.price
  end
  min_price = FetchPrice(sProduct) or 0
  prices[sProduct] = { price = min_price, time = timeNow }
  return min_price
end

function GetSkill(skill)
  local s = skills[skill]
  if s == nil then
    print("Missing skill " .. skill)
    return { level = default_skill }
  end
  return s
end

function GetInventionParams(sProduct, nDecryptor, nMeta, bMaxRuns)
  local nProbability, tDecryptor, tProduct, nCost, nRuns, nME, nPE, nPerUnit, tInvention,
         tBaseProduct

  tInvention = invention[sProduct]
  if not tInvention then
    return { prob = 1, cost = 0, runs = 1, me = materials[sProduct].me, pe = 0, perUnit = 0 }
  end
  tProduct = materials[sProduct]
  tDecryptor = decryptors[nDecryptor]
  nProbability = tProduct.basechance
  nProbability = nProbability * (1 + 0.01 * GetSkill(tInvention.encryption.skill).level)
  nProbability = nProbability * (1 + 0.02 * (GetSkill(tInvention.scientific1.skill).level
                                           + GetSkill(tInvention.scientific2.skill).level) *
                                             5 / (5 - nMeta))
  nProbability = nProbability * tDecryptor.probability
  nCost = GetPrice(tInvention.scientific1.material) * tInvention.scientific1.quantity +
          GetPrice(tInvention.scientific2.material) * tInvention.scientific2.quantity
  nCost = nCost + GetPrice(tDecryptor[tInvention.encryption.decryptors])
  tBaseProduct = materials[tProduct.baseBlueprint]
  if bMaxRuns then
    nRuns = tBaseProduct.maxRuns
  else
    nRuns = 1
  end
  nRuns = math.floor(nRuns / tBaseProduct.maxRuns * tProduct.maxRuns / 10 + tDecryptor.runs)
  nRuns = math.max(nRuns, 1)
  nRuns = math.min(nRuns, tProduct.maxRuns)
  nME = -4 + tDecryptor.me
  nPE = -4 + tDecryptor.pe
  nPerUnit = nCost / (nProbability * nRuns)
  return { prob = nProbability, cost = nCost, runs = nRuns, me = nME, pe = nPE, perUnit = nPerUnit }
end

function GetBestParameters(sProduct, nMeta, bconsiderReactions, bMaxRun)
  local nBest, nBestCost, tParams, nCost, nBestRuns, nMaxRuns
  nBestCost = -1
  nMaxRuns = bMaxRun and 1 or 0
  for nRuns = 0, nMaxRuns do
    for i, tDecryptor in ipairs(decryptors) do
      tParams = GetInventionParams(sProduct, i, nMeta, nRuns > 0)
      nCost = GetMaterialCost(ResolveMaterials(sProduct, tParams.me, bConsiderReactions))
      nCost = nCost + tParams.perUnit
      if nBestCost < 0  or nBestCost > nCost then
        nBestCost = nCost
        nBest = i
        nBestRuns = nRuns
      end
    end
  end
  return { decryptor = nBest, maxRuns = nBestRuns > 0 }
end

function GetDecryptor(me, runs)
  for i = 1, #decryptors do
    if decryptors[i].me == me and (runs == nil or decryptors[i].runs == runs) then
      return i
    end
  end
  return 0
end

function PrintInventionParams(tParams)
  print("Success probability: " .. tParams.prob)
  print("Cost per attempt: " .. tParams.cost)
  print("Runs per result: " .. tParams.runs)
  print("ME of resulting BPC: " .. tParams.me)
  print("PE of resulting BPC: " .. tParams.pe)
  print("Average cost per unit: " .. tParams.perUnit)
end

function GetMEWaste(nME, nBaseWaste)
  if nME >= 0 then
    return nBaseWaste / (1 + nME)
  else
    return nBaseWaste * (-nME + 1)
  end
end

function TranslateME(nMaterial, nOrigME, nNewME, nBaseWaste)
  local nOrigWaste
--  nOrigWaste = nOrigME and GetMEWaste(nOrigME, nBaseWaste) or 0
  nOrigWaste = 0
  return round(nMaterial * (1 + GetMEWaste(nNewME, nBaseWaste))
          / (1 + nOrigWaste))
end

function ResolveReaction(sProduct, iNumber, tMaterials)
  local nScale, tReaction
  tReaction = reactions[sProduct]
  if tReaction == nil then
    return nil
  end
  nScale = iNumber / tReaction.outputs[sProduct];
  for m, q in pairs(tReaction.inputs) do
    local nQuantity, nOutput
    nOutput = tReaction.outputs[m] or 0
    nQuantity = (q - nOutput) * nScale
    ResolveMaterials(m, nil, true, nQuantity, tMaterials)
  end
  return tMaterials
end

function ResolveMaterials(sProduct, nME, bConsiderReactions, iNumber, tMaterials)
  if iNumber == nil then
    iNumber = 1
  end
  if tMaterials == nil then
    tMaterials = {}
  end
  local tBlueprint = materials[sProduct]
  if tBlueprint == nil then
    if bConsiderReactions and ResolveReaction(sProduct, iNumber, tMaterials) then
      return tMaterials
    end
    local iMat = tMaterials[sProduct] or 0
    tMaterials[sProduct] = iMat + iNumber
    return tMaterials
  end
  if nME == nil then
    nME = tBlueprint.me
  end
  for sMat, iCount in pairs(tBlueprint.materials) do
    local nQuant, tMat
    tMat = materials[sMat]
    if string.sub(sMat, 1, 6) ~= "R.A.M." and (not tMat or not tMat.shipGroup) then
      nQuant = TranslateME(iCount, tBlueprint.me, nME, tBlueprint.basewaste)
    else
      nQuant = iCount
    end
    ResolveMaterials(sMat, nil, bConsiderReactions, nQuant * iNumber, tMaterials)
  end
  return tMaterials
end

function GetMaterialCost(tMaterials)
  local nTotal = 0
  for sMat, iCount in pairs(tMaterials) do
    local nPrice = GetPrice(sMat)
    nPrice = nPrice * iCount
    nTotal = nTotal + nPrice
  end
  return nTotal
end

function PrintMaterials(tMaterials, nInvention)
  local nTotal = 0
  for sMat, iCount in pairs(tMaterials) do
    local nPrice = GetPrice(sMat)
    if nPrice == nil then
      print("Missing price for " .. sMat)
      nPrice = 0
    end
    nPrice = nPrice * iCount
    nTotal = nTotal + nPrice
    print(sMat .. " X " .. iCount .. " = " .. nPrice)
  end
  print("\nMaterials per unit: " .. nTotal)
  print("Invention cost per unit: " .. nInvention)
  nTotal = nTotal + nInvention
  print("\nTotal: " .. nTotal)
end


function GetRowFields(fields)
  local row = {}
  for i = 0, fields.Count - 1 do
    local fld = fields:Item(i)
    if fld and fld.Value then
      row[fld.Name]=fld.Value
    end
  end
  return row
end

function LoadInputs()
  LoadTypeIDs()
  LoadInventionData()
  LoadDecryptors()
  LoadBlueprints()
  LoadMaterials()
  LoadShipGroups()
  LoadReactions()
  LoadPriceXML()
end

function SaveOutputs()
  SavePriceXML()
end

function LoadDataFromADO()
  require "luasql.ado"

  local conn, res, cur, row

  env = luasql.ado()
  conn = env:connect("ebs_DATADUMP", "sa", "saPass")
  conn:setautocommit(false)

  cur = conn:execute([[
    select typeID, typeName
    from dbo.invTypes
    where marketGroupID is not null
  ]])
  row = cur:fetch({}, "a")
  while row do
    print(row.typeName, row.typeID)
    row = cur:fetch (row, "a")
  end

  cur:close()
  conn:close()
  env:close()
end

function PrintResult(result)
  if invention[result.product] then
    print("Best decryptor to invent " .. result.product .. " is: " .. decryptors[result.decr.decryptor][invention[result.product].encryption.decryptors])
    if result.decr.maxRuns then
      print("It's best to use max run BPC")
    end
  end
  PrintInventionParams(result.inv)
  print()
  PrintMaterials(result.mats, result.inv.perUnit)
  print(result.product .. " market price is: " .. tostring(result.market))
  print("Profit per unit: ", result.profit)
end

function CalcProfit(product, considerReactions, considerMaxRuns, decryptorME, decryptorRuns)
  local result

  result = {}
  result.product = product

  if decryptorME ~= nil then
    result.decr = { decryptor = GetDecryptor(decryptorME, decryptorRuns), maxRuns = considerMaxRuns }
  else
    result.decr = GetBestParameters(product, 0, considerReactions, considerMaxRuns)
  end

  result.inv = GetInventionParams(product, result.decr.decryptor, 0, result.decr.maxRuns)
  result.mats = ResolveMaterials(product, result.inv.me, considerReactions)
  result.market = GetPrice(product)
  result.materialCost = GetMaterialCost(result.mats)
  result.profit = result.market - result.materialCost - result.inv.perUnit

  return result
end

function InsertSort(array, predicate)
  local i, j, t
  for i = 2, #array do
    j = i
    t = array[i]
    while j > 1 and predicate(t, array[j - 1]) do
      array[j] = array[j - 1]
      j = j - 1
    end
    if i ~= j then
      array[j] = t
    end
  end
end

function PadStr(str, iLen)
  while #str < iLen do
    str = str .. " "
  end
  return str
end

function Dump(var, indent)
  if indent == nil then
    indent = 0
  end
  indent = indent
  if type(var) == "table" then
    print("{")
    for k, v in pairs(var) do
      for i = 1, indent + 2 do
        io.write(' ')
      end
      io.write(k, " = ")
      Dump(v, indent + 2)
    end
    for i = 1, indent do
      io.write(' ')
    end
    print("}")
  else
    print(var)
  end
end

function SortProfits(considerReactions, considerMaxRuns)
  local ships, results, i, fnCompare

  ships = {
    "Purifier", "Guardian", "Pilgrim", "Curse", "Zealot", "Sacrilege", "Heretic", "Devoter", "Anathema", "Damnation", "Absolution",
    "Manticore", "Basilisk", "Falcon", "Rook", "Cerberus", "Eagle", "Flycatcher", "Onyx", "Buzzard", "Vulture", "Nighthawk",
    "Nemesis", "Oneiros", "Arazu", "Lachesis", "Ishtar", "Deimos", "Eris", "Phobos", "Helios", "Eos", "Astarte",
    "Hound", "Scimitar", "Rapier", "Huginn", "Vagabond", "Muninn", "Sabre", "Broadsword", "Cheetah", "Claymore", "Sleipnir",
  }

  results = {}

  for i = 1, #ships do
    results[i] = CalcProfit(ships[i], considerReactions, considerMaxRuns)
    if materials[ships[i]].basechance == 0.3 then
      results[i].number = 10 -- a frigate
    else
      results[i].number = 1
    end
  end

  fnCompare = function(a, b)
                return a.profit * a.number < b.profit * b.number
              end

  InsertSort(results, fnCompare)

  for i = 1, #results do
    print(PadStr(results[i].product, 15) ..
          PadStr(decryptors[results[i].decr.decryptor][invention[results[i].product].encryption.decryptors], 24) ..
          PadStr(results[i].decr.maxRuns and "Max" or "non-Max", 10) ..
          PadStr(tostring(results[i].number), 6) ..
          results[i].profit * results[i].number)
  end
end

function AggregateMaterials(tProducts)
  local sProduct, iNumber, tMats, i, tProd
  tMats = {}
  for i, tProd in ipairs(tProducts) do
    tMats = ResolveMaterials(tProd.name, tProd.me, tProd.react, tProd.num, tMats)
  end
  return tMats
end

function PrintAllMats(considerReactions)
  local tProducts
  tProducts = {
    { name = "Huginn", me = -3, num = 1, react = considerReactions },
    { name = "Onyx", me = -2, num = 1, react = considerReactions },
    { name = "Zealot", me = -3, num = 1, react = considerReactions },
    { name = "Devoter", me = -2, num = 1, react = considerReactions },
    { name = "Broadsword", me = -1, num = 1, react = considerReactions },
  }
  PrintMaterials(AggregateMaterials(tProducts), 0)
end

--LoadDataFromADO()

LoadInputs()

--Dump(reactions["Pure Standard Drop Booster"])
--Dump(materials["Standard Drop Booster"])

--PrintResult(CalcProfit("Strong Sooth Sayer Booster", true))

SortProfits(false, true)
--PrintResult(CalcProfit("Large Energy Discharge Elutriation II", false, true))
--PrintResult(CalcProfit("1200mm Artillery Cannon II", false, true))
--PrintResult(CalcProfit("Scimitar", false, false, 2))
--PrintResult(CalcProfit("Onyx", false, true))
--PrintMaterials(ResolveMaterials("Scimitar", -6, 1), 0, false)
--PrintAllMats(false)

SaveOutputs()


