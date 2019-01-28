Vec3 CAMERA = {0.0 ,0.5, 3.0};
Vec3 LIGHT = {0.4, 3.0, 3.0};

RECT BoundingBox(Vec3 v0, Vec3 v1, Vec3 v2) {
  RECT r;
  r.left = min(v0.x, min(v1.x, v2.x));
  r.right = max(v0.x, max(v1.x, v2.x));
  r.bottom = min(v0.y, min(v1.y, v2.y));
  r.top = max(v0.y, max(v1.y, v2.y));
  return r; 
}

// It seems funky that p has always z = 0 but if we 
// carry out the  computation youll realize it's ignored
// baricentric coords act on the triangle as it was a plane.
// has no notion of the z component.
Vec3 Baricenter(Vec3 p, Vec3 a, Vec3 b, Vec3 c) {
  Vec3 AB = v3_sub(b,a);
  Vec3 AC = v3_sub(c,a);
  Vec3 PA = v3_sub(a,p);

  Vec3 X = {AB.x, AC.x, PA.x};
  Vec3 Y = {AB.y, AC.y, PA.y};

  Vec3 UV1 = v3_cross(X,Y);

  float u = UV1.x / UV1.z;
  float v = UV1.y / UV1.z;
  
  return {1-u-v, u, v};
}
bool PointInTriangle(Vec3 bari) {

  float u = bari.y;
  float v = bari.z;
  return u >= 0 && v >= 0 && (u + v) <= 1;
}
mat4_t LookAt(Vec3 eye, Vec3 center, Vec3 up) {
  Vec3 z = v3_norm(v3_sub(eye, center));
  Vec3 x = v3_norm(v3_cross(up, z));
  Vec3 y = v3_norm(v3_cross(z, x));
  return mat4(
    x.x, x.y, x.z, 0.0,
    y.x, y.y, y.z, 0.0,
    z.x, z.y, z.z, 0.0,
    0.0, 0.0, 0.0, 1.0);
}

mat4_t ViewPort(Bitmap bitmap) {
  float w = (float) bitmap.width * 3/4;
  float h = (float) bitmap.height * 3/4;
  float depth = 255.0;
  mat4_t scale = mat4(
  w/2.0, 0.0, 0.0, 0.0,
  0.0, h/2.0, 0.0, 0.0,
  0.0, 0.0, depth/2, 0.0,
  0.0, 0.0, 0.0, 1.0);

  mat4_t translate = mat4(
  1.0, 0.0, 0.0, 100+w/2.0,
  0.0, 1.0, 0.0, 100+h/2.0,
  0.0, 0.0, 1.0, depth/2.0,
  0.0, 0.0, 0.0, 1.0);
  return m4_mul(translate, scale);
}

float PERSPECTIVE;
mat4_t Perspective() {
  
  return  mat4(
  1.0, 0.0,    0.0, 0.0,
  0.0, 1.0,    0.0, 0.0,
  0.0, 0.0,    1.0, 0.0,
  0.0, 0.0,    PERSPECTIVE, 1.0);
}

mat4_t ModelView(Vec3 eye) {
  return LookAt(eye, {0, 0 ,0}, {0, 1, 0});
}
mat4_t PMV = m4_mul(Perspective(), ModelView(CAMERA));
mat4_t PMVIT;
mat4_t TO_SHADOW;
void SetupCamera(Vec3 eye) {
  PMV = m4_mul(Perspective(), ModelView(eye));
  PMVIT = m4_transpose(m4_invert(PMV));
}

Vec3 GetNormalFromTexture(Vec3 texture) {
  int tex_x = (int) (texture.x * TEXTURE_NM.width);
  int tex_y = (int) (texture.y * TEXTURE_NM.height);
  Color c = TEXTURE_NM.buffer[tex_x + tex_y * TEXTURE_NM.width];

  Vec3 normal = {
  // scale to [0,1], scale to [0,2], transpose to [-1,1]
  c.red/255.0f * 2.0f -1.0f,
  c.green/255.0f * 2.0f -1.0f,
  c.blue/255.0f * 2.0f -1.0f,
  };
  return v3_norm(m4_mul_dir(PMVIT, normal));
}

Color FragmentShader(Bitmap bitmap, Vec3 bari, mat4_t texture_matrix, mat4_t vertex_matrix) {
  Vec3 vertice = m4_mul_pos(vertex_matrix, bari);
  Vec3 shadow_screenspace = m4_mul_pos(TO_SHADOW, vertice);
  int shadow_idx = (int)(shadow_screenspace.x)  + (int)(shadow_screenspace.y) * bitmap.width;
  float shadow = 1.0;
  if (shadow_screenspace.z < bitmap.shadow_buffer[shadow_idx] - 8) {
    shadow = 0.1;
  }

  Vec3 texture = m4_mul_dir(texture_matrix, bari);
  int tex_x = (int) (texture.x * TEXTURE_DIFFUSE.width);
  int tex_y = (int) (texture.y * TEXTURE_DIFFUSE.height);

  Vec3 n = GetNormalFromTexture(texture);
  Vec3 l = v3_norm(m4_mul_pos(PMV, v3_norm(LIGHT)));
  float diffuse = max(0.0, v3_dot(l, n));
  Color c = TEXTURE_DIFFUSE.buffer[tex_x + tex_y * TEXTURE_DIFFUSE.width];

  Color spec_color = TEXTURE_SPEC.buffer[tex_x + tex_y * TEXTURE_SPEC.width];
  float spec_power_blue = spec_color.blue;
  float spec_power_green = spec_color.green;
  float spec_power_red = spec_color.red;
  Vec3 r = v3_norm(v3_sub(v3_scale(n, (v3_dot(n, l) * 2)), l));
  // we only need to take z because by this point everything is in camera space
  // Z is aligned with the camera by now.
  int extra = 2;
  float specular_blue = pow(max(r.z, 0.0), spec_power_blue + extra );
  float specular_green = pow(max(r.z, 0.0), spec_power_green + extra);
  float specular_red = pow(max(r.z, 0.0), spec_power_red + extra);

  BYTE ambient = 6;
  float intensity_blue =  1.0 * diffuse + 0.5 * specular_blue;
  float intensity_green =  1.0 * diffuse + 0.5 * specular_green;
  float intensity_red =  1.0 * diffuse + 0.5 * specular_red;
  Color c2 = {
    min(ambient + (BYTE)(c.blue * intensity_blue * shadow), 255), 
    min(ambient + (BYTE)(c.green * intensity_green * shadow), 255), 
    min(ambient + (BYTE)(c.red * intensity_red * shadow), 255), 
    0 };
  return c2;
}
Color VoidShader(Bitmap bitmap, Vec3 bari, mat4_t texture_matrix, mat4_t) {
  return {255, 255, 255, 255};
}

typedef Color (*FragShaderFunc)(Bitmap, Vec3, mat4_t, mat4_t);

void DrawTriangle(Bitmap bitmap, Vec3 v0, Vec3 v1, Vec3 v2, 
                  mat4_t texture_matrix, mat4_t vertex_matrix, FragShaderFunc frag_shader) {
  RECT r = BoundingBox(v0, v1, v2);
  for(int i = r.left; i <= r.right; i++) {
    for(int j = r.bottom; j <= r.top; j++) {
      Vec3 bari = Baricenter(Vec3{(float)i, (float)j, 0.0}, v0,v1,v2);
      if(PointInTriangle(bari)) {
        float z = v0.z * bari.x + v1.z * bari.y + v2.z * bari.z;
        if(bitmap.z_buffer[i + j * bitmap.width] < z) {
          bitmap.z_buffer[i + j * bitmap.width]  = z;
          Color c = frag_shader(bitmap, bari, texture_matrix, vertex_matrix);
          bitmap.buffer[i + j * bitmap.width] = c.ToHex();
        }
      }
    }
  }
}
void DrawFace(Bitmap bitmap, FragShaderFunc frag_shader)  {
  for (int i = 0; i < MODEL.faces_size; i++) {
    Face* face = &MODEL.faces[i];
    int v_index0 = face->vertice_indexes[0]-1;
    int v_index1 = face->vertice_indexes[1]-1;
    int v_index2 = face->vertice_indexes[2]-1;

    Vec3 world_v0 = MODEL.vertices[v_index0];
    Vec3 world_v1 = MODEL.vertices[v_index1];
    Vec3 world_v2 = MODEL.vertices[v_index2];

    Vec3 t0 = MODEL.vertice_textures[face->vertice_texture_indexes[0]-1];
    Vec3 t1 = MODEL.vertice_textures[face->vertice_texture_indexes[1]-1];
    Vec3 t2 = MODEL.vertice_textures[face->vertice_texture_indexes[2]-1];


    mat4_t texture_matrix =
      mat4(t0.x, t1.x, t2.x, 0.0,
           t0.y, t1.y, t2.y, 0.0,
            0.0,  0.0, 0.0, 0.0,
            0.0,  0.0, 0.0, 0.0);
    Vec3 world_verts[] = {world_v0, world_v1, world_v2};
    Vec3 screen_verts[3];
    for (int j = 0; j < 3; j++) {
      Vec3 screen_vert = m4_mul_pos(m4_mul(ViewPort(bitmap), PMV), world_verts[j]);
      screen_verts[j] = screen_vert;
    }
    mat4_t vertex_matrix =
      mat4(
      screen_verts[0].x, screen_verts[1].x, screen_verts[2].x, 0.0,
      screen_verts[0].y, screen_verts[1].y, screen_verts[2].y, 0.0,
      screen_verts[0].z, screen_verts[1].z, screen_verts[2].z, 0.0,
            0.0,  0.0, 0.0, 0.0);
    DrawTriangle(bitmap, screen_verts[0], screen_verts[1], 
                 screen_verts[2], texture_matrix, vertex_matrix, frag_shader);
  }
}

void Draw(Bitmap bitmap) {
  PERSPECTIVE = -1.0/v3_length(LIGHT);
  SetupCamera(LIGHT);
  DrawFace(bitmap, &VoidShader);
  float* tmp = bitmap.z_buffer;
  bitmap.z_buffer = bitmap.shadow_buffer;
  bitmap.shadow_buffer = tmp;
  memset(bitmap.buffer, 0, sizeof(DWORD) * bitmap.width * bitmap.height);
  mat4_t vpmv_shadow = m4_mul(ViewPort(bitmap), PMV);
  PERSPECTIVE = -1.0/v3_length(CAMERA);
  SetupCamera(CAMERA);
  mat4_t vpmv_camera = m4_mul(ViewPort(bitmap), PMV);
  TO_SHADOW = m4_mul(vpmv_shadow, m4_invert(vpmv_camera));
  DrawFace(bitmap, &FragmentShader);
}
