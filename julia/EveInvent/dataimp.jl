import ODBC
import YAML

function import_decryptors()
	ODBC.connect("esb_DATADUMP")
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
	# ODBC.DataFrames.writetable("data/Decryptors.csv", res)
	decID = -1
	decryptors = {}
	for row = 1:size(res, 1)
		if decID != res[row, :typeID]
			decID = res[row, :typeID]
			dec = Dict{String, Any}()
			dec["id"] = decID
			dec["name"] = res[row, :typeName]
			dec["attributes"] = Dict{String, Float64}()
			push!(decryptors, dec)
		end
		dec["attributes"][res[row, :attributeName]] = res[row, :valueFloat]
	end
	json_write("data/decryptors.json", decryptors)
end

function import_blueprints()
	# because loading a json is so much faster than loading a yaml for some reason
	blueprints = YAML.load_file("data/blueprints.yaml")
	json_write("data/blueprints.json", blueprints; indent=0)
end

