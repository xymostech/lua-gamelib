local gl = require 'gl'
local util = require 'util'

local vertex_data = {
  0.25,  0.25, -1.25, 1.0,
  0.25, -0.25, -1.25, 1.0,
 -0.25,  0.25, -1.25, 1.0,

  0.25, -0.25, -1.25, 1.0,
 -0.25, -0.25, -1.25, 1.0,
 -0.25,  0.25, -1.25, 1.0,

  0.25,  0.25, -2.75, 1.0,
 -0.25,  0.25, -2.75, 1.0,
  0.25, -0.25, -2.75, 1.0,

  0.25, -0.25, -2.75, 1.0,
 -0.25,  0.25, -2.75, 1.0,
 -0.25, -0.25, -2.75, 1.0,

 -0.25,  0.25, -1.25, 1.0,
 -0.25, -0.25, -1.25, 1.0,
 -0.25, -0.25, -2.75, 1.0,

 -0.25,  0.25, -1.25, 1.0,
 -0.25, -0.25, -2.75, 1.0,
 -0.25,  0.25, -2.75, 1.0,

  0.25,  0.25, -1.25, 1.0,
  0.25, -0.25, -2.75, 1.0,
  0.25, -0.25, -1.25, 1.0,

  0.25,  0.25, -1.25, 1.0,
  0.25,  0.25, -2.75, 1.0,
  0.25, -0.25, -2.75, 1.0,

  0.25,  0.25, -2.75, 1.0,
  0.25,  0.25, -1.25, 1.0,
 -0.25,  0.25, -1.25, 1.0,

  0.25,  0.25, -2.75, 1.0,
 -0.25,  0.25, -1.25, 1.0,
 -0.25,  0.25, -2.75, 1.0,

  0.25, -0.25, -2.75, 1.0,
 -0.25, -0.25, -1.25, 1.0,
  0.25, -0.25, -1.25, 1.0,

  0.25, -0.25, -2.75, 1.0,
 -0.25, -0.25, -2.75, 1.0,
 -0.25, -0.25, -1.25, 1.0,
}

local color_data = {
  0.0, 0.0, 1.0, 1.0,
  0.0, 0.0, 1.0, 1.0,
  0.0, 0.0, 1.0, 1.0,

  0.0, 0.0, 1.0, 1.0,
  0.0, 0.0, 1.0, 1.0,
  0.0, 0.0, 1.0, 1.0,

  0.8, 0.8, 0.8, 1.0,
  0.8, 0.8, 0.8, 1.0,
  0.8, 0.8, 0.8, 1.0,

  0.8, 0.8, 0.8, 1.0,
  0.8, 0.8, 0.8, 1.0,
  0.8, 0.8, 0.8, 1.0,

  0.0, 1.0, 0.0, 1.0,
  0.0, 1.0, 0.0, 1.0,
  0.0, 1.0, 0.0, 1.0,

  0.0, 1.0, 0.0, 1.0,
  0.0, 1.0, 0.0, 1.0,
  0.0, 1.0, 0.0, 1.0,

  0.5, 0.5, 0.0, 1.0,
  0.5, 0.5, 0.0, 1.0,
  0.5, 0.5, 0.0, 1.0,

  0.5, 0.5, 0.0, 1.0,
  0.5, 0.5, 0.0, 1.0,
  0.5, 0.5, 0.0, 1.0,

  1.0, 0.0, 0.0, 1.0,
  1.0, 0.0, 0.0, 1.0,
  1.0, 0.0, 0.0, 1.0,

  1.0, 0.0, 0.0, 1.0,
  1.0, 0.0, 0.0, 1.0,
  1.0, 0.0, 0.0, 1.0,

  0.0, 1.0, 1.0, 1.0,
  0.0, 1.0, 1.0, 1.0,
  0.0, 1.0, 1.0, 1.0,

  0.0, 1.0, 1.0, 1.0,
  0.0, 1.0, 1.0, 1.0,
  0.0, 1.0, 1.0, 1.0,
}

function setup_data(vertex_buffer)
  gl.with_buffer(
    gl.ARRAY_BUFFER, vertex_buffer,
    function()
      gl.buffer_data(
        gl.ARRAY_BUFFER,
        (#vertex_data + #color_data) * 8, gl.STATIC_DRAW
      )

      gl.buffer_sub_float_data(gl.ARRAY_BUFFER, 0, vertex_data)

      gl.buffer_sub_float_data(gl.ARRAY_BUFFER, #vertex_data * 8, color_data)
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
  local vertex_shader = gl.create_shader_from_file(gl.VERTEX_SHADER, "vertex.shader")
  local fragment_shader = gl.create_shader_from_file(gl.FRAGMENT_SHADER, "fragment.shader")

  local program = gl.create_program_from_shaders({vertex_shader, fragment_shader})

  gl.delete_shader(vertex_shader)
  gl.delete_shader(fragment_shader)

  local vertex_array = gl.create_vertex_array()
  gl.bind_vertex_array(vertex_array)

  local vertex_buffer = gl.create_buffer_object()
  setup_data(vertex_buffer)

  local offset_uniform = gl.get_uniform_location(program, "offset")

  gl.enable(gl.CULL_FACE)
  gl.cull_face(gl.BACK)
  gl.front_face(gl.CW)

  local perspective_uniform = gl.get_uniform_location(program, "perspective_matrix")

  gl.with_program(
    program,
    function()
      gl.uniform_matrix_float(
        perspective_uniform, 4, 4,
        make_perspective_matrix(1, 0.5, 3.0)
      )
    end
  )

  local data = {
    counter = 1,
    program = program,
    vertex_array = vertex_array,
    vertex_buffer = vertex_buffer,
    offset_uniform = offset_uniform,
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
      gl.uniform_float(
        data.offset_uniform,
        {math.sin(data.counter / 10), math.cos(data.counter / 10)}
      )

      gl.with_buffer(
        gl.ARRAY_BUFFER, data.vertex_buffer,
        function()
          gl.with_attribs(
            {0, 1},
            function()
              gl.vertex_attrib_pointer(0, 4, gl.DOUBLE, gl.FALSE, 0, 0)
              gl.vertex_attrib_pointer(1, 4, gl.DOUBLE, gl.FALSE, 0, #vertex_data * 8)

              gl.draw_arrays(gl.TRIANGLES, 0, #vertex_data / 4)
            end
          )
        end
      )
    end
  )

  gl.swap_window()
end
