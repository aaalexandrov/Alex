#!/usr/bin/perl -w
use strict;
use OpenGL qw/ :all /;
use Time::HiRes qw (time);

my $offset_x = 4;
my $offset_y = 4;
my $cell_size = 16;
my @cell_color = ( 1.0, 0.0, 1.0, 1.0 );

my $field_width = 10;
my $field_height = 30;
my @field;

my $d_near = -1.0;
my $d_far = 1.0;
my $mode = GLUT_DOUBLE;
my @clear_color = ( 0.0, 0.0, 0.0, 1.0 );
my $width = $cell_size * $field_width + 2 * $offset_x;
my $height = $cell_size * $field_height + 2 * $offset_y;

my $part;
my $part_size = 4;
my $part_x;
my $part_y;
my $part_rot;
my $part_time;

my $game_running = 1;
my $game_base_speed = 0.1;
my $game_drop_speed = 0.02;
my $game_speed = $game_base_speed;

my @parts = (
  [ [ 0, 1, 0, 0 ],
    [ 1, 1, 1, 0 ],
    [ 0, 0, 0, 0 ],
    [ 0, 0, 0, 0 ] ],

  [ [ 1, 1, 0, 0 ],
    [ 0, 1, 1, 0 ],
    [ 0, 0, 0, 0 ],
    [ 0, 0, 0, 0 ] ],

  [ [ 0, 1, 1, 0 ],
    [ 1, 1, 0, 0 ],
    [ 0, 0, 0, 0 ],
    [ 0, 0, 0, 0 ] ],
    
  [ [ 1, 0, 0, 0 ],
    [ 1, 1, 1, 0 ],
    [ 0, 0, 0, 0 ],
    [ 0, 0, 0, 0 ] ],

  [ [ 0, 0, 1, 0 ],
    [ 1, 1, 1, 0 ],
    [ 0, 0, 0, 0 ],
    [ 0, 0, 0, 0 ] ],

  [ [ 0, 0, 0, 0 ],
    [ 1, 1, 1, 1 ],
    [ 0, 0, 0, 0 ],
    [ 0, 0, 0, 0 ] ],

  [ [ 1, 1, 0, 0 ],
    [ 1, 1, 0, 0 ],
    [ 0, 0, 0, 0 ],
    [ 0, 0, 0, 0 ] ],
);

my @part_sizes = ( 2, 2, 2, 2, 2, 3, 1 );

sub initField {
  my ( $i, $j );
  $#field = 0;
  for $i (0 .. $field_height - 1) {
    $field[$i] = ();
    for $j (0 .. $field_width - 1) {
      $field[$i][$j] = 0;
    }
  }
  $game_running = 1;
  spawnPart();
}

sub drawField {
  my $i;
  my $j;
  for ($i = 0; $i <= $#field; $i++) {
    for ($j = 0; $j <= $#{$field[$i]}; $j++) {
      if ($field[$i][$j]) {
        glColor4f(@cell_color);
      } else {
        glColor4f(@clear_color);
      }
      my $x = $cell_size * $j + $offset_x;
      my $y = $cell_size * $i + $offset_y;
      glRectf($x, $y, $x + $cell_size, $y + $cell_size);
    }
  }
}

sub rotateCoords {
  my ( $x, $y ) = @_;
  my $size = $part_sizes[$part];
  
  if (!$part_rot) {
    return ( $x, $y );
  } elsif ($part_rot == 1) {
    return ( $size - $y, $x );
  } elsif ($part_rot == 2) {
    return ( $size - $x, $size - $y );
  } elsif ($part_rot == 3) {
    return ( $y, $size - $x );
  }
}

sub rotatePart {
  my ( $i, $j );
  my @res = ();
  
  for $i (0 .. $part_size - 1) {
    $res[$i] = ();
    for $j (0 .. $part_size - 1) {
      my ( $x, $y ) = rotateCoords($i, $j);
#      print STDERR "($i $j) => ($x $y) ";
      $res[$i][$j] = $parts[$part][$x][$y];
    }
  }
  return \@res;
}

sub checkPart {
  my ( $p ) = @_;
  my ( $i, $j );
  for $i (0 .. $part_size - 1) {
    for $j (0 .. $part_size - 1) {
      my $x = $j + $part_x;
      my $y = $i + $part_y;
      if ($p->[$i][$j] && ($x < 0 || $x >= $field_width || $y < 0 || $y >= $field_height || $field[$y][$x])) {
        return 0;
      }
    }
  }
  return 1;
}

sub applyPart {
  my ( $p, $val ) =  @_;

  my ( $i, $j );
  for $i (0 .. $part_size - 1) {
    for $j (0 .. $part_size - 1) {
      my $x = $j + $part_x;
      my $y = $i + $part_y;
      if ($p->[$i][$j]) {
        $field[$y][$x] = $val;
      }
    }
  }
}

sub compactLines {
  my ( $i, $j );
  $i = 0;
  while ($i < $field_height) {
    my $solid = 1;
    for ($j = 0; $solid && $j < $field_width; $j++) {
      if (!$field[$i][$j]) {
        $solid = 0;
      }
    }
    if ($solid) {
      for $j ($i .. $field_height - 2) {
        $field[$j] = $field[$j + 1];
      }
      $field[$field_height - 1] = ();
      for $j (0 .. $field_width - 1) {
        $field[$field_height - 1][$j] = 0;
      }
    } else {
      $i++;
    }
  }
}

sub spawnPart {
  compactLines();
  $game_speed = $game_base_speed;
  $part_time = time();
  $part = int(rand($#parts + 1));
  $part_x = ($field_width - $part_sizes[$part]) / 2;
  $part_y = $field_height - $part_sizes[$part] - 1;
  $part_rot = 0;
  my $p = rotatePart();
  if (!checkPart($p)) {
    $game_running = 0;
  } else {
    applyPart($p, 1);
  }
}

sub movePart {
  my $p = rotatePart();
  applyPart($p, 0);
  $part_y--;
  if (checkPart($p)) {
    applyPart($p, 1);
    $part_time = time();
  } else {
    $part_y++;
    applyPart($p, 1);
    spawnPart();
  }
}

sub oneFrame {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if ($game_running && time() - $part_time >= $game_speed) {
    movePart();
  }

  drawField();

  if ($mode == GLUT_SINGLE) {
    glFlush(); 
  }
  else {
    glutSwapBuffers();
  }
}

sub display {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

sub myReshape {
  my ($w, $h)= @_;
  $width = $w;
  $height = $h;
  glViewport(0, 0, $w, $h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
#  print STDERR "$w x $h ";
  glOrtho(0, $w - 1, 0, $h - 1, $d_near, $d_far);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

sub visibility {
  my $status = shift;
  if ($status == GLUT_VISIBLE) {
    glutIdleFunc(\&oneFrame);
  }
  else {
    glutIdleFunc(undef);
  }
}

sub myinit {
  glClearColor(@clear_color);
  myReshape($width, $height);
  # glShadeModel(GL_FLAT);
  # glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  initField();
}

sub keyboard {
  my ($key, $x, $y) = @_;
  exit(0) if $key == 27;
  if ($key == 32) {
    initField();
  }
}

sub keySpecial {
  my ($key, $x, $y) = @_;
  my $delta = 0;
  if ($key == GLUT_KEY_LEFT) {
    $delta = -1;
  } elsif ($key == GLUT_KEY_RIGHT) {
    $delta = 1;
  } elsif ($key == GLUT_KEY_UP) {
    my $p = rotatePart();
    applyPart($p, 0);
    $part_rot = ($part_rot + 1) % 4;
    my $q = rotatePart();
    if (checkPart($q)) {
      applyPart($q, 1);
    } else {
      $part_rot = ($part_rot + 3) % 4;
      applyPart($p, 1);
    }
  } elsif ($key == GLUT_KEY_DOWN) {
    $game_speed = $game_drop_speed;
  }
  if ($delta) {
    my $p = rotatePart();
    applyPart($p, 0);
    $part_x += $delta;
    if (!checkPart($p)) {  
	  $part_x -= $delta;
    }
    applyPart($p, 1);
  }
}

glutInit();

# if (argc > 1)   $mode = GLUT_SINGLE;
glutInitDisplayMode($mode | GLUT_RGB | GLUT_DEPTH);
glutInitWindowPosition(100, 100);
glutInitWindowSize($width, $height);
glutCreateWindow("Test");

myinit();
glutReshapeFunc(\&myReshape);
glutDisplayFunc(\&display);
glutKeyboardFunc(\&keyboard);
glutSpecialFunc(\&keySpecial);
glutVisibilityFunc(\&visibility);
glutPostRedisplay();
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
glutMainLoop();

__END__