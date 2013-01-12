select tr.typeName, r.input, t.typeName, r.quantity 
from invTypeReactions r, invTypes tr, invTypes t
where r.reactionTypeID = tr.typeID
  and r.typeID = t.typeID
--  and tr.typeName like '%Exile Booster%'
order by tr.typeName