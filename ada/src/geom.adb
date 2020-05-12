with Ada.Numerics.Generic_Elementary_Functions;
with Ada.Float_Text_IO;
with Ada.Strings;
with Ada.Strings.Fixed;

package body Geom is

  package Float_Funcs is new Ada.Numerics.Generic_Elementary_Functions(Float);
  use Float_Funcs;

  package Float_IO renames Ada.Float_Text_IO;

  use Ada.Strings;
  use Ada.Strings.Fixed;

  function "+"(l, r: in Point) return Point is
  begin
    return (x => l.x+r.x, y => l.y+r.y);
  end "+";

  function "-"(l, r: in Point) return Point is 
  begin
    return (x => l.x-r.x, y => l.y-r.y);
  end "-";

  function "*"(p: in Point; f: in Float) return Point is
  begin
    return (x => p.x*f, y => p.y*f);
  end "*";
    
  function "*"(f: in Float; p: in Point) return Point is
  begin
    return p*f;
  end "*";

  function "/"(p: in Point; f: in Float) return Point is
  begin
    return (x => p.x/f, y => p.y/f);
  end "/";

  function Magnitude(p: in Point) return Float is
  begin
    return (p.x**2 + p.y**2)**0.5;
  end Magnitude;

  function Distance(a, b: in Point) return Float is
  begin
    return Magnitude(a-b);
  end Distance;
   
  function Image(p: in Point) return String is
    x: String(1..10);
    y: String(1..10);
  begin
    Float_IO.Put(x, p.x, 2, 0);
    Float_IO.Put(y, p.y, 2, 0);

    return "(x: " & Trim(x, Left) & ", y: " & Trim(y, Left) & ")";
  end Image;

end Geom;
