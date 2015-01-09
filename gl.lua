local M = {}

-- OpenGL, misc. functions exposed from C
copy_funcs = {
  glClearColor="clear_color",
  glClear="clear",

  glDrawArrays="draw_arrays",
  glDrawElements="draw_elements",

  glEnableVertexAttribArray="enable_vertex_attrib_array",
  glDisableVertexAttribArray="disable_vertex_attrib_array",
  glVertexAttribPointer="vertex_attrib_pointer",

  CreateShaderFromFile="create_shader_from_file",
  glDeleteShader="delete_shader",
  CreateProgramFromShaders="create_program_from_shaders",
  glDeleteProgram="delete_program",
  glUseProgram="use_program",

  CreateBufferObject="create_buffer_object",
  DeleteBufferObject="delete_buffer_object",
  glBindBuffer="bind_buffer",
  glBufferData="buffer_data",
  BufferSubDoubleData="buffer_sub_double_data",
  BufferSubUnsignedIntData="buffer_sub_unsigned_int_data",

  CreateVertexArray="create_vertex_array",
  DeleteVertexArray="delete_vertex_array",
  glBindVertexArray="bind_vertex_array",

  glGetUniformLocation="get_uniform_location",
  glUniformFloat="uniform_float",
  glUniformMatrixFloat="uniform_matrix_float",

  glEnable="enable",
  glDisable="disable",

  glCullFace="cull_face",
  glFrontFace="front_face",

  SDL_GL_SwapWindow="swap_window",
}

for gl_name, lua_name in pairs(copy_funcs) do
  M[lua_name] = _G["draw_" .. gl_name]
end

-- OpenGL constants exposed from C
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

  "BLEND",
  "CLIP_DISTANCE0",
  "CLIP_DISTANCE1",
  "CLIP_DISTANCE2",
  "CLIP_DISTANCE3",
  "CLIP_DISTANCE4",
  "CLIP_DISTANCE5",
  "CLIP_DISTANCE6",
  "CLIP_DISTANCE7",
  "COLOR_LOGIC_OP",
  "CULL_FACE",
  "DEBUG_OUTPUT",
  "DEBUG_OUTPUT_SYNCHRONOUS",
  "DEPTH_CLAMP",
  "DEPTH_TEST",
  "DITHER",
  "FRAMEBUFFER_SRGB",
  "LINE_SMOOTH",
  "MULTISAMPLE",
  "POLYGON_OFFSET_FILL",
  "POLYGON_OFFSET_LINE",
  "POLYGON_OFFSET_POINT",
  "POLYGON_SMOOTH",
  "PRIMITIVE_RESTART",
  "PRIMITIVE_RESTART_FIXED_INDEX",
  "RASTERIZER_DISCARD",
  "SAMPLE_ALPHA_TO_COVERAGE",
  "SAMPLE_ALPHA_TO_ONE",
  "SAMPLE_COVERAGE",
  "SAMPLE_SHADING",
  "SAMPLE_MASK",
  "SCISSOR_TEST",
  "STENCIL_TEST",
  "TEXTURE_CUBE_MAP_SEAMLESS",
  "PROGRAM_POINT_SIZE",

  "FRONT",
  "BACK",
  "FRONT_AND_BACK",

  "CW",
  "CCW",
}

for _, name in ipairs(consts) do
  M[name] = _G["draw_GL_" .. name]
end

-- Extra functions exposed
function M.with_vertex_array(vao, func)
  M.bind_vertex_array(vao)

  func()

  M.bind_vertex_array(nil)
end

function M.with_program(program, func)
  M.use_program(program)

  func()

  M.use_program(nil)
end

function M.with_attribs(attribs, func)
  for _, attrib in ipairs(attribs) do
    M.enable_vertex_attrib_array(attrib)
  end

  func()

  for _, attrib in ipairs(attribs) do
    M.disable_vertex_attrib_array(attrib)
  end
end

function M.with_buffer(target, buffer, func)
  M.bind_buffer(target, buffer)

  func()

  M.bind_buffer(target, nil)
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
