select t.typeName, ISNULL(a.valueFloat, a.valueInt), at.attributeName, g.groupName
from dgmTypeAttributes a, invTypes t, dgmAttributeTypes at,
     invGroups g, invCategories c
where a.typeID = t.typeID 
  and a.attributeID = at.attributeID
  and t.groupID = g.groupID 
  and t.published = 1
  and g.categoryID = c.categoryID 
  and c.categoryName = 'Decryptors'
order by typeName, attributeName