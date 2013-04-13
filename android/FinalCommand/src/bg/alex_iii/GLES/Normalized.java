package bg.alex_iii.GLES;

import java.lang.annotation.*; 

@Retention(RetentionPolicy.RUNTIME)
@Target(ElementType.FIELD)
public @interface Normalized {
  // Indicates that a vertex format data field should be normalized
}
