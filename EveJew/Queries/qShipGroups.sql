select t.typeName, g.groupName
from invTypes t, invGroups g, invCategories c
where t.groupID = g.groupID
  and g.categoryID = c.categoryID
  and c.categoryName = 'Ship'
  and t.marketGroupID is not NULL
  and t.published != 0
order by g.groupName, t.typeName