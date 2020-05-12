package Geom is

  type Point is record
    x, y: Float;
  end record;
    
  Zero: constant Point := (others=>0.0);
  One:  constant Point := (others=>1.0);

  function "+"(l, r: in Point) return Point;
  function "-"(l, r: in Point) return Point;

  function "*"(p: in Point; f: in Float) return Point;
  function "*"(f: in Float; p: in Point) return Point;

  function "/"(p: in Point; f: in Float) return Point;

  function Magnitude(p: in Point) return Float;
  function Distance(a, b: in Point) return Float;

  function Image(p: in Point) return String;
end Geom;
