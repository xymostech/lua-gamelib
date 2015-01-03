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

  offset_uniform = gl.get_uniform_location(program, "offset")

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
  gl.clear_color(data.counter / 1000, 0.0, 0.0, 1.0);
  gl.clear(gl.COLOR_BUFFER_BIT)

  gl.use_program(data.program)

  gl.uniform_float(data.offset_uniform, {math.sin(data.counter / 10), math.cos(data.counter / 10)})

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
