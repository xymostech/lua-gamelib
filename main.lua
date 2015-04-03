local gl = require 'gl'
local util = require 'util'

local obj1_vertex_data = {
  2.0, 1.0, 0.0, 1.0,
  2.0, 0.0, -1.0, 1.0,
  2.0, -1.0, 0.0, 1.0,
  2.0, 0.0, 1.0, 1.0,

  -2.0, 1.0, 0.0, 1.0,
  -2.0, 0.0, -1.0, 1.0,
  -2.0, -1.0, 0.0, 1.0,
  -2.0, 0.0, 1.0, 1.0,
}

local obj2_vertex_data = {
  1.0, 2.0, 0.0, 1.0,
  0.0, 2.0, -1.0, 1.0,
 -1.0, 2.0, 0.0, 1.0,
  0.0, 2.0, 1.0, 1.0,

  1.0, -2.0, 0.0, 1.0,
  0.0, -2.0, -1.0, 1.0,
 -1.0, -2.0, 0.0, 1.0,
  0.0, -2.0, 1.0, 1.0,
}

local color_data = {
  0.0, 0.0, 1.0, 1.0,
  0.0, 0.0, 1.0, 1.0,
  0.0, 0.0, 1.0, 1.0,
  0.0, 0.0, 1.0, 1.0,

  1.0, 0.0, 0.0, 1.0,
  1.0, 0.0, 0.0, 1.0,
  1.0, 0.0, 0.0, 1.0,
  1.0, 0.0, 0.0, 1.0,
}

--    7-----4
--   /|    /|
--  / 6---/-5
-- 3-----0 /
-- |/    |/
-- 2-----1
local index_data = {
  0, 1, 2,
  2, 3, 0,

  4, 7, 6,
  6, 5, 4,

  0, 4, 5,
  5, 1, 0,

  7, 3, 2,
  2, 6, 7,

  7, 4, 0,
  0, 3, 7,

  2, 1, 5,
  5, 6, 2,
}

local gl_type_data = {
  [gl.DOUBLE] = {
    size = 8,
    data_func = gl.buffer_sub_double_data,
  },

  [gl.UNSIGNED_INT] = {
    size = 4,
    data_func = gl.buffer_sub_unsigned_int_data,
  },
}

function write_arrays_to_buffer(buffer, data_type, ...)
  local size = 0
  local type_data = gl_type_data[data_type]

  for i, array in ipairs({...}) do
    size = size + (#array * type_data.size)
  end

  print(size)

  gl.buffer_data(buffer, size, gl.STATIC_DRAW)

  local position = 0

  for i, array in ipairs({...}) do
    print(i, position)
    type_data.data_func(buffer, position, array)
    position = position + (#array * type_data.size)
  end
end

function setup_data(vertex_buffer, index_buffer)
  gl.with_buffer(
    gl.ARRAY_BUFFER, vertex_buffer,
    function()
      write_arrays_to_buffer(
        gl.ARRAY_BUFFER,
        gl.DOUBLE,
        obj1_vertex_data, obj2_vertex_data,
        color_data, color_data
      )
    end
  )

  gl.with_buffer(
    gl.ELEMENT_ARRAY_BUFFER, index_buffer,
    function()
      write_arrays_to_buffer(gl.ELEMENT_ARRAY_BUFFER, gl.UNSIGNED_INT, index_data)
    end
  )
end

function make_perspective_matrix(frustum_scale, z_near, z_far)
  return {
    frustum_scale, 0, 0, 0,
    0, frustum_scale, 0, 0,
    0, 0, (z_near + z_far) / (z_near - z_far), -1,
    0, 0, (2 * z_near * z_far) / (z_near - z_far), 0
  }
end

function startup()
  local vertex_shader = gl.create_shader_from_file(gl.VERTEX_SHADER, "main.vertex.glsl")
  local fragment_shader = gl.create_shader_from_file(gl.FRAGMENT_SHADER, "main.fragment.glsl")

  local program = gl.create_program_from_shaders({vertex_shader, fragment_shader})

  gl.delete_shader(vertex_shader)
  gl.delete_shader(fragment_shader)

  local vertex_buffer = gl.create_buffer_object()
  local index_buffer = gl.create_buffer_object()
  setup_data(vertex_buffer, index_buffer)

  local vertex_array = gl.create_vertex_array()
  gl.with_vertex_array(
    vertex_array,
    function()
      gl.bind_buffer(gl.ARRAY_BUFFER, vertex_buffer)

      gl.enable_vertex_attrib_array(0)
      gl.vertex_attrib_pointer(0, 4, gl.DOUBLE, gl.FALSE, 0, 0)

      gl.enable_vertex_attrib_array(1)
      gl.vertex_attrib_pointer(
        1, 4, gl.DOUBLE, gl.FALSE, 0, (#obj1_vertex_data + #obj2_vertex_data) * 8)

      gl.bind_buffer(gl.ELEMENT_ARRAY_BUFFER, index_buffer)
    end
  )

  gl.enable(gl.CULL_FACE)
  gl.cull_face(gl.BACK)
  gl.front_face(gl.CW)

  local perspective_uniform = gl.get_uniform_location(program, "perspective_matrix")
  local offset_uniform = gl.get_uniform_location(program, "offset")

  gl.with_program(
    program,
    function()
      gl.uniform_matrix_float(
        perspective_uniform, 4, 4,
        make_perspective_matrix(1, 0.5, 10.0)
      )
    end
  )

  local data = {
    counter = 1,
    program = program,
    vertex_array = vertex_array,
    vertex_buffer = vertex_buffer,
    index_buffer = index_buffer,
    uniforms = {
      offset = offset_uniform
    }
  }
  return data
end

function cleanup(data)
  gl.delete_program(data.program)

  gl.delete_vertex_array(data.vertex_array)
  gl.delete_buffer_object(data.vertex_buffer)
end

function update(data)
  local new_data = util.extend(
    {}, data,
    {
      counter = data.counter + 1
    }
  )
  return new_data, (data.counter >= 1000)
end

function render(data)
  gl.clear_color(0.0, 0.0, 0.0, 1.0)
  gl.clear(gl.COLOR_BUFFER_BIT)

  gl.with_program(
    data.program,
    function()
      gl.with_vertex_array(
        data.vertex_array,
        function()
          gl.uniform_float(
            data.uniforms.offset, {0, 0, -4}
          )

          gl.draw_elements_base_vertex(
            gl.TRIANGLES, #index_data, gl.UNSIGNED_INT, 0, 0)

          gl.uniform_float(
            data.uniforms.offset, {0, 0, -6}
          )

          gl.draw_elements_base_vertex(
            gl.TRIANGLES, #index_data, gl.UNSIGNED_INT, 0, 8)
        end
      )
    end
  )

  gl.swap_window()
end
