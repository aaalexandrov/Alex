select t.typeName, t1.typeName, am.quantity, ts.typeName, 
      (select gs.groupName
       from dgmTypeAttributes ta1, dgmAttributeTypes at1, invGroups gs
       where ta1.typeID = am.requiredTypeID
         and ta1.attributeID = at1.attributeID
         and at1.attributeName = 'decryptorID'
         and ta1.valueInt = gs.groupID)

from invMetaTypes mt, invTypes t, invTypes tp, invMetaGroups mg, invBlueprintTypes b, invBlueprintTypes bp,
     ramTypeRequirements am, ramActivities a, invTypes t1, invMarketGroups m,
     dgmTypeAttributes ta, dgmAttributeTypes at, invTypes ts
     
where mt.metaGroupID = mg.metaGroupID
  and mg.metaGroupName = 'Tech II'
  and b.productTypeID = mt.typeID
  and bp.productTypeID = mt.parentTypeID
  and t.typeID = b.productTypeID
  and tp.typeID = bp.blueprintTypeID
--  and t.typeName = 'Sentinel'
  and t1.marketGroupID = m.marketGroupID
  
  and ta.typeID = am.requiredTypeID
  and ta.attributeID = at.attributeID
  and at.attributeName = 'requiredSkill1'
  and ta.valueInt = ts.typeID

  and bp.blueprintTypeID = am.typeID
  and am.requiredTypeID = t1.typeID
  and am.activityID = a.activityID
  and a.activityName like 'Invention'
order by t.typeName, am.quantity

