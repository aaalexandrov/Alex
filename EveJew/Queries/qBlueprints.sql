select tp.typeName, b.wasteFactor, b.maxProductionLimit, b.techLevel, 
       (select t1.typeName
        from invMetaGroups mg, invMetaTypes mt,
             invBlueprintTypes b1, invTypes t1
        where mt.typeID = tp.typeID
          and mt.parentTypeID = b1.productTypeID
          and b1.productTypeID = t1.typeID
          and mg.metaGroupID = mt.metaGroupID)
from invBlueprintTypes b, invTypes t, invTypes tp
where b.productTypeID = tp.typeID
  and b.blueprintTypeID = t.typeID
  and t.published != 0
  and tp.published != 0
order by tp.typeName

