struct Bitmap {
  DWORD* buffer;
  float* z_buffer;
  float* shadow_buffer;
  LONG width;
  LONG height;
};
#define abs(x)  (((x)<0)?-(x):(x))
#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))

struct Face {
  int vertice_indexes[3];
  int vertice_texture_indexes[3];
};
typedef struct { float x, y, z; } Vec3;
struct Model {
  Vec3 vertices[4048];
  int vertices_size;
  Vec3 vertice_normals[4048];
  int vertice_normals_size;
  Vec3 vertice_textures[4048];
  int vertice_textures_size;
  Face faces[6000];
  int faces_size;
};
Model MODEL;

struct Color {
  BYTE blue;
  BYTE green;
  BYTE red;
  BYTE alpha;
  DWORD ToHex() {
    return *(DWORD*)this;
  }
};

struct Texture {
  short width;
  short height;
  unsigned char bpp;
  Color* buffer;
};

typedef union {
	// The first index is the column index, the second the row index. The memory
	// layout of nested arrays in C matches the memory layout expected by OpenGL.
	float m[4][4];
	// OpenGL expects the first 4 floats to be the first column of the matrix.
	// So we need to define the named members column by column for the names to
	// match the memory locations of the array elements.
	struct {
		float m00, m01, m02, m03;
		float m10, m11, m12, m13;
		float m20, m21, m22, m23;
		float m30, m31, m32, m33;
	};
} mat4_t;

static inline Vec3 v3_scale   (Vec3 a, float b)          { return { a.x * b, a.y * b, a.z * b}; }
static inline Vec3 v3_add   (Vec3 a, Vec3 b)          { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
static inline Vec3 v3_adds  (Vec3 a, float s)           { return { a.x + s,   a.y + s,   a.z + s   }; }
static inline Vec3 v3_sub   (Vec3 a, Vec3 b)          { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
static inline Vec3 v3_subs  (Vec3 a, float s)           { return { a.x - s,   a.y - s,   a.z - s   }; }
static inline Vec3 v3_mul   (Vec3 a, Vec3 b)          { return { a.x * b.x, a.y * b.y, a.z * b.z }; }
static inline Vec3 v3_muls  (Vec3 a, float s)           { return { a.x * s,   a.y * s,   a.z * s   }; }
static inline Vec3 v3_div   (Vec3 a, Vec3 b)          { return { a.x / b.x, a.y / b.y, a.z / b.z }; }
static inline Vec3 v3_divs  (Vec3 a, float s)           { return { a.x / s,   a.y / s,   a.z / s   }; }
static inline float  v3_length (Vec3 v)                    { return sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);}
static inline float  v3_dot   (Vec3 a, Vec3 b)          { return a.x*b.x + a.y*b.y + a.z*b.z; }
static inline Vec3 v3_norm(Vec3 v) {
	float len = v3_length(v);
	if (len > 0)
		return { v.x / len, v.y / len, v.z / len };
	else
		return { 0, 0, 0};
}
static inline Vec3 v3_cross(Vec3 a, Vec3 b) {
	return {
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

static inline mat4_t mat4(
	float m00, float m10, float m20, float m30,
	float m01, float m11, float m21, float m31,
	float m02, float m12, float m22, float m32,
	float m03, float m13, float m23, float m33
) {
  return {
	m00, m01, m02, m03,
	m10, m11, m12, m13,
	m20, m21, m22, m23,
	m30, m31, m32, m33};
};

static inline mat4_t m4_mul(mat4_t a, mat4_t b) {
	mat4_t result;
	
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 4; j++) {
			float sum = 0;
			for(int k = 0; k < 4; k++) {
				sum += a.m[k][j] * b.m[i][k];
			}
			result.m[i][j] = sum;
		}
	}
	
	return result;
}
static inline mat4_t m4_invert(mat4_t m) {
	mat4_t result;
	
  float A2323 = m.m22 * m.m33 - m.m23 * m.m32;
  float A1323 = m.m21 * m.m33 - m.m23 * m.m31;
  float A1223 = m.m21 * m.m32 - m.m22 * m.m31;
  float A0323 = m.m20 * m.m33 - m.m23 * m.m30;
  float A0223 = m.m20 * m.m32 - m.m22 * m.m30;
  float A0123 = m.m20 * m.m31 - m.m21 * m.m30;
  float A2313 = m.m12 * m.m33 - m.m13 * m.m32;
  float A1313 = m.m11 * m.m33 - m.m13 * m.m31;
  float A1213 = m.m11 * m.m32 - m.m12 * m.m31;
  float A2312 = m.m12 * m.m23 - m.m13 * m.m22;
  float A1312 = m.m11 * m.m23 - m.m13 * m.m21;
  float A1212 = m.m11 * m.m22 - m.m12 * m.m21;
  float A0313 = m.m10 * m.m33 - m.m13 * m.m30;
  float A0213 = m.m10 * m.m32 - m.m12 * m.m30;
  float A0312 = m.m10 * m.m23 - m.m13 * m.m20;
  float A0212 = m.m10 * m.m22 - m.m12 * m.m20;
  float A0113 = m.m10 * m.m31 - m.m11 * m.m30;
  float A0112 = m.m10 * m.m21 - m.m11 * m.m20;

  float det = m.m00 * ( m.m11 * A2323 - m.m12 * A1323 + m.m13 * A1223 ) 
    - m.m01 * ( m.m10 * A2323 - m.m12 * A0323 + m.m13 * A0223 ) 
    + m.m02 * ( m.m10 * A1323 - m.m11 * A0323 + m.m13 * A0123 ) 
    - m.m03 * ( m.m10 * A1223 - m.m11 * A0223 + m.m12 * A0123 ) ;

  det = 1 / det;

   result.m00 = det *   ( m.m11 * A2323 - m.m12 * A1323 + m.m13 * A1223 );
   result.m01 = det * - ( m.m01 * A2323 - m.m02 * A1323 + m.m03 * A1223 );
   result.m02 = det *   ( m.m01 * A2313 - m.m02 * A1313 + m.m03 * A1213 );
   result.m03 = det * - ( m.m01 * A2312 - m.m02 * A1312 + m.m03 * A1212 );
   result.m10 = det * - ( m.m10 * A2323 - m.m12 * A0323 + m.m13 * A0223 );
   result.m11 = det *   ( m.m00 * A2323 - m.m02 * A0323 + m.m03 * A0223 );
   result.m12 = det * - ( m.m00 * A2313 - m.m02 * A0313 + m.m03 * A0213 );
   result.m13 = det *   ( m.m00 * A2312 - m.m02 * A0312 + m.m03 * A0212 );
   result.m20 = det *   ( m.m10 * A1323 - m.m11 * A0323 + m.m13 * A0123 );
   result.m21 = det * - ( m.m00 * A1323 - m.m01 * A0323 + m.m03 * A0123 );
   result.m22 = det *   ( m.m00 * A1313 - m.m01 * A0313 + m.m03 * A0113 );
   result.m23 = det * - ( m.m00 * A1312 - m.m01 * A0312 + m.m03 * A0112 );
   result.m30 = det * - ( m.m10 * A1223 - m.m11 * A0223 + m.m12 * A0123 );
   result.m31 = det *   ( m.m00 * A1223 - m.m01 * A0223 + m.m02 * A0123 );
   result.m32 = det * - ( m.m00 * A1213 - m.m01 * A0213 + m.m02 * A0113 );
   result.m33 = det *   ( m.m00 * A1212 - m.m01 * A0212 + m.m02 * A0112 );

	return result;
}
static inline mat4_t m4_transpose(mat4_t matrix) {
	return mat4(
		matrix.m00, matrix.m01, matrix.m02, matrix.m03,
		matrix.m10, matrix.m11, matrix.m12, matrix.m13,
		matrix.m20, matrix.m21, matrix.m22, matrix.m23,
		matrix.m30, matrix.m31, matrix.m32, matrix.m33
	);
}
Vec3 m4_mul_pos(mat4_t matrix, Vec3 position) {
	Vec3 result = {
		matrix.m00 * position.x + matrix.m10 * position.y + matrix.m20 * position.z + matrix.m30,
		matrix.m01 * position.x + matrix.m11 * position.y + matrix.m21 * position.z + matrix.m31,
		matrix.m02 * position.x + matrix.m12 * position.y + matrix.m22 * position.z + matrix.m32
  };	
	float w = matrix.m03 * position.x + matrix.m13 * position.y + matrix.m23 * position.z + matrix.m33;
	if (w != 0 && w != 1)
		return {result.x / w, result.y / w, result.z / w};
	
	return result;
}

Vec3 m4_mul_dir(mat4_t matrix, Vec3 direction) {
	Vec3 result = {
		matrix.m00 * direction.x + matrix.m10 * direction.y + matrix.m20 * direction.z + matrix.m30,
		matrix.m01 * direction.x + matrix.m11 * direction.y + matrix.m21 * direction.z + matrix.m31,
		matrix.m02 * direction.x + matrix.m12 * direction.y + matrix.m22 * direction.z + matrix.m32
  };	
	float w = matrix.m03 * direction.x + matrix.m13 * direction.y + matrix.m23 * direction.z;
	if (w != 0 && w != 1)
		return {result.x / w, result.y / w, result.z / w};
	
	return result;
}
