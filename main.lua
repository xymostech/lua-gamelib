local gl = require 'gl'
local util = require 'util'

function setup_data(vertex_buffer)
  gl.bind_buffer(gl.ARRAY_BUFFER, vertex_buffer)

  gl.buffer_data(gl.ARRAY_BUFFER, 4 * 6 * 8, gl.STATIC_DRAW)

  gl.buffer_sub_float_data(gl.ARRAY_BUFFER, 0, {
     0.75,  0.75, 0.0, 1.0,
     0.75, -0.75, 0.0, 1.0,
    -0.75, -0.75, 0.0, 1.0,
     1.0,   0.0,  0.0, 1.0,
     0.0,   1.0,  0.0, 1.0,
     0.0,   0.0,  1.0, 1.0,
  })

  gl.bind_buffer(gl.ARRAY_BUFFER, nil)
end

function startup()
  vertex_shader = gl.create_shader_from_file(gl.VERTEX_SHADER, "vertex.shader")
  fragment_shader = gl.create_shader_from_file(gl.FRAGMENT_SHADER, "fragment.shader")

  program = gl.create_program_from_shaders({vertex_shader, fragment_shader})

  gl.delete_shader(vertex_shader)
  gl.delete_shader(fragment_shader)

  vertex_array = gl.create_vertex_array()
  gl.bind_vertex_array(vertex_array)

  vertex_buffer = gl.create_buffer_object()
  setup_data(vertex_buffer)

  local data = {
    counter = 1,
    program = program,
    vertex_array = vertex_array,
    vertex_buffer = vertex_buffer,
  }
  return data
end

function update_thread_update(data)
  local new_data = util.extend({}, data, {
    counter = data.counter + 1
  })
  return new_data, (data.counter >= 100)
end

function render_thread_render(data)
  gl.clear_color(data.counter / 1000, 0.0, 0.0, 1.0);
  gl.clear(gl.COLOR_BUFFER_BIT)

  gl.use_program(data.program)

  gl.bind_buffer(gl.ARRAY_BUFFER, data.vertex_buffer)

  gl.enable_vertex_attrib_array(0);
  gl.enable_vertex_attrib_array(1);
  gl.vertex_attrib_pointer(0, 4, gl.DOUBLE, gl.FALSE, 0, 0)
  gl.vertex_attrib_pointer(1, 4, gl.DOUBLE, gl.FALSE, 0, 4 * 3 * 8)

  gl.draw_arrays(gl.TRIANGLES, 0, 3)

  gl.disable_vertex_attrib_array(0)
  gl.disable_vertex_attrib_array(1)

  gl.bind_buffer(gl.ARRAY_BUFFER, nil)

  gl.use_program(nil)

  gl.swap_window()
end
