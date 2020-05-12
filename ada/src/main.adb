with Ada.Text_IO; use Ada.Text_IO;
with Geom; use Geom;
with Ada.Numerics.Generic_Real_Arrays;


procedure Main is
  package Float_Arrays is new Ada.Numerics.Generic_Real_Arrays(Float);
  use Float_Arrays;

  p0: Point := (x=>2.0, y=>3.0);
  d: Float;
  v3: Real_Vector(1..3) := (1.0, 2.0, 3333333333.0);
begin
  d := Distance(p0, Zero);

  Put("Distance between p0 and zero is: "); Put_Line(d'Image);

  Put("Distance between zero and one is: "); Put_Line(Distance(Zero, One)'Image);

  Put_Line(Image(p0 + One));

  Put_Line("(" & v3(1)'Image & "," & v3(2)'Image & "," & v3(3)'Image & ")");
end Main;
