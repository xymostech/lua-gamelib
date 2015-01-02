local M = {}

copy_funcs = {
  glClearColor="clear_color",
  glClear="clear",
  glUseProgram="use_program",
  glDrawArrays="draw_arrays",
  glEnableVertexAttribArray="enable_vertex_attrib_array",
  glDisableVertexAttribArray="disable_vertex_attrib_array",
  glVertexAttribPointer="vertex_attrib_pointer",
  CreateShaderFromFile="create_shader_from_file",
  glDeleteShader="delete_shader",
  CreateProgramFromShaders="create_program_from_shaders",
  glDeleteProgram="delete_program",
  CreateBufferObject="create_buffer_object",
  glBindBuffer="bind_buffer",
  glBufferData="buffer_data",
  BufferSubFloatData="buffer_sub_float_data",
  CreateVertexArray="create_vertex_array",
  glBindVertexArray="bind_vertex_array",
  glGetUniformLocation="get_uniform_location",
  glUniformFloat="uniform_float",
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

  "COMPUTE_SHADER",
  "VERTEX_SHADER",
  "TESS_CONTROL_SHADER",
  "TESS_EVALUATION_SHADER",
  "GEOMETRY_SHADER",
  "FRAGMENT_SHADER",

  "ARRAY_BUFFER",
  "ATOMIC_COUNTER_BUFFER",
  "COPY_READ_BUFFER",
  "COPY_WRITE_BUFFER",
  "DISPATCH_INDIRECT_BUFFER",
  "DRAW_INDIRECT_BUFFER",
  "ELEMENT_ARRAY_BUFFER",
  "PIXEL_PACK_BUFFER",
  "PIXEL_UNPACK_BUFFER",
  "QUERY_BUFFER",
  "SHADER_STORAGE_BUFFER",
  "TEXTURE_BUFFER",
  "TRANSFORM_FEEDBACK_BUFFER",
  "UNIFORM_BUFFER",

  "STREAM_DRAW",
  "STREAM_READ",
  "STREAM_COPY",
  "STATIC_DRAW",
  "STATIC_READ",
  "STATIC_COPY",
  "DYNAMIC_DRAW",
  "DYNAMIC_READ",
  "DYNAMIC_COPY",
}

for _, name in ipairs(consts) do
  M[name] = _G["draw_GL_" .. name]
end

setmetatable(
  M,
  {
    __index = function(_, value)
      print("Error: Tried to access unknown opengl value:", value)
      return nil
    end
  }
)

return M
