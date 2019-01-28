int ParseVertices(FILE* file, Vec3 * vertices) {
  int count = 0;
  char buffer[1024];
  rewind(file);
    while (fgets(buffer, sizeof buffer, file)) {
    if (!(buffer[0] == 'v' && buffer[1] == ' ')) continue;
    float x, y, z;
    sscanf(buffer, "%*c %f %f %f", &x, &y, &z);  
    vertices[count] = Vec3 {x, y, z};
    count++;
  }
  return count;
}

int ParseVerticeNormals(FILE* file, Vec3* vertice_normals) {
  int count = 0;
  char buffer[1024];
  rewind(file);
    while (fgets(buffer, sizeof buffer, file)) {
    if (!(buffer[0] == 'v' && buffer[1] == 'n' && buffer[2] == ' ')) continue;
    float x, y, z;
    sscanf(buffer, "%*c%*c  %f %f %f", &x, &y, &z);  
    vertice_normals[count] = Vec3 {x, y, z};
    count++;
  }
  return count;
}

int ParseVerticeTextures(FILE* file, Vec3* vertice_textures) {
  int count = 0;
  char buffer[1024];
  rewind(file);
    while (fgets(buffer, sizeof buffer, file)) {
    if (!(buffer[0] == 'v' && buffer[1] == 't' && buffer[2] == ' ')) continue;
    float x, y, z;
    sscanf(buffer, "%*c%*c  %f %f %f", &x, &y, &z);  
    vertice_textures[count] = Vec3 {x, y, z};
    count++;
  }
  return count;
}

int ParseFaces(FILE* file, Face* faces) {
  int count = 0;
  char buffer[1024];
  rewind(file);
  while (fgets(buffer, sizeof buffer, file)) {
    if (!(buffer[0] == 'f' && buffer[1] == ' ')) continue;

    int v1, v2, v3;
    int t1, t2, t3;
    sscanf (buffer, "%*c %d/%d/%*d %d/%d/%*d %d/%d/%*d",  &v1, &t1, &v2, &t2, &v3, &t3);
    faces[count] = Face {{v1, v2, v3}, {t1, t2, t3}};
    count++;
  }
  return count;
}


void ParseObjFile(const char* file_name) {
  FILE* file = fopen(file_name, "r");
  if(!file) {
    printf("ERROR OPENING FILE %s", file_name);
  }
  MODEL.vertices_size = ParseVertices(file, MODEL.vertices);
  MODEL.vertice_normals_size = ParseVerticeNormals(file, MODEL.vertice_normals);
  MODEL.vertice_textures_size = ParseVerticeTextures(file, MODEL.vertice_textures);
  MODEL.faces_size = ParseFaces(file, MODEL.faces);
  fclose(file);
}

void ParseHeader(FILE* file, Texture* texture) {
  fseek(file, 12, SEEK_SET);  
  // make sure to check that padding does not fuck up
  // this clever reading of w, h and bpp.
  fread(texture, 5, 1, file);
}

void ParseTgaRle(FILE* file, Texture* texture) {
  int bitmap_position = 0;
  bool has_alpha = texture->bpp == 32;
  bool has_color = texture->bpp != 8;
  fseek(file, 18, SEEK_SET);  
  do {
    unsigned char rep_count = fgetc(file);
    if(rep_count == 0xFF) {
      int x = 1234;
    }
    // raw packets
    if(rep_count < 128) {
      rep_count++;
      for(int i = 0; i < rep_count; i++) {
        BYTE b = fgetc(file);
        BYTE g = 0;
        BYTE r = 0;
        BYTE a = 0;
        if(has_color) {
          g = fgetc(file);
          r = fgetc(file);
        }
        if(has_alpha) {
          a = fgetc(file);
        }
        texture->buffer[bitmap_position++]  = {b, g, r, a};
      }
    } else {
      rep_count -= 127; 
      BYTE b = fgetc(file);
      BYTE g = 0;
      BYTE r = 0;
      BYTE a = 0;
      if(has_color) {
        g = fgetc(file);
        r = fgetc(file);
      }
      if(has_alpha) {
        a = fgetc(file);
      }
      for(int i = 0; i < rep_count; i++) {
        texture->buffer[bitmap_position++]  = {b, g, r, a};
      }
    }
    
  }while(bitmap_position < texture->width * texture->height);
}

void ParseTgaImage(const char* file_name, Texture* texture) {
  FILE* file = fopen(file_name, "rb");
  ParseHeader(file, texture);
  texture->buffer = (Color*) malloc(sizeof Color * 
                                    texture->width *
                                    texture->height);
  ParseTgaRle(file, texture);

  fclose(file);
}

