(
select t2.typeName, t1.typeName, am.quantity, am.damagePerJob
from ramTypeRequirements am, invTypes t, invTypes t1, ramActivities a,
     invBlueprintTypes b, invTypes t2

--, invMarketGroups m, invMarketGroups m1
where am.typeID = t.typeID 
  and am.requiredTypeID = t1.typeID 
  and am.activityID = a.activityID
  and b.productTypeID = t2.typeID
--  and t2.typeName = 'Zealot'
  and am.typeID = b.blueprintTypeID
  and am.damagePerJob > 0
  and am.quantity > 0
  and t2.published != 0
--  and t.typeName like 'Zealot Blueprint'
  and a.activityName like 'Manufacturing'
--  and t1.marketGroupID = m.marketGroupID
--  and m1.marketGroupID = m.parentGroupID
--  and m1.marketGroupName != 'Skills'
) 
union
(
select t2.typeName, tm.typeName, quantity, 1

from invBlueprintTypes b, invTypeMaterials m, invTypes t2, invTypes tm

where b.productTypeID = m.typeID
  and b.productTypeID = t2.typeID
  and m.materialTypeID = tm.typeID
)
order by t2.typeName