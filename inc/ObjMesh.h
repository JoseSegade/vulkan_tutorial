#ifndef INC_OBJMESH_H_
#define INC_OBJMESH_H_

#include "Common.h"
#include "Util.h"
#include <unordered_map>

namespace vkMesh {

class ObjMesh {
 public:
  std::vector<float>                         vertices;
  std::vector<Index>                         indices;
  std::unordered_map<std::string, uint32_t>  history;
  std::unordered_map<std::string, glm::vec3> colors;
  glm::vec3                                  brushColor;
  std::vector<glm::vec3>                     v;
  std::vector<glm::vec3>                     vn;
  std::vector<glm::vec2>                     vt;
  glm::mat4                                  preTransform;

  ObjMesh(
    const char* objFilepath,
    const char* mtlFilepath,
    glm::mat4 preTransform
  );

  void read_vertex_data(const std::vector<std::string>& words);
  void read_texcoord_data(const std::vector<std::string>& words);
  void read_normal_data(const std::vector<std::string>& words);
  void read_face_data(const std::vector<std::string>& words);
  void read_corner(const std::string& words);
};

}

#endif  // INC_OBJMESH_H_

