local M = {}

copy_funcs = {
  glClearColor="clear_color",
  glClear="clear",
  glUseProgram="use_program",
  glDrawArrays="draw_arrays",
  glEnableVertexAttribArray="enable_vertex_attrib_array",
  glDisableVertexAttribArray="disable_vertex_attrib_array",
  glVertexAttribPointer="vertex_attrib_pointer",
  SDL_GL_SwapWindow="swap_window",
}

for gl_name, lua_name in pairs(copy_funcs) do
  M[lua_name] = _G["draw_" .. gl_name]
end

consts = {
  "COLOR_BUFFER_BIT",
  "DEPTH_BUFFER_BIT",
  "STENCIL_BUFFER_BIT",

  "POINTS",
  "LINE_STRIP",
  "LINE_LOOP",
  "LINES",
  "LINE_STRIP_ADJACENCY",
  "LINES_ADJACENCY",
  "TRIANGLE_STRIP",
  "TRIANGLE_FAN",
  "TRIANGLES",
  "TRIANGLE_STRIP_ADJACENCY",
  "TRIANGLES_ADJACENCY",
  "PATCHES",

  "TRUE",
  "FALSE",

  "BYTE",
  "UNSIGNED_BYTE",
  "SHORT",
  "UNSIGNED_SHORT",
  "INT",
  "UNSIGNED_INT",
  "HALF_FLOAT",
  "FLOAT",
  "DOUBLE",
  "FIXED",
  "INT_2_10_10_10_REV",
  "UNSIGNED_INT_2_10_10_10_REV",
  "UNSIGNED_INT_10F_11F_11F_REV",
}

for _, name in ipairs(consts) do
  M[name] = _G["draw_GL_" .. name]
end

return M
