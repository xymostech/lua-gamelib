local gl = require 'gl'

function startup()
  local data = {a=1}
  return data
end

function update_thread_update(data)
  local new_data = {a = data.a + 1}
  return new_data, (data.a >= 1000)
end

function render_thread_render(data)
  gl.clear_color(data.a / 1000, 0.0, 0.0, 1.0);
  gl.clear(gl.COLOR_BUFFER_BIT)

  c_draw()

  gl.enable_vertex_attrib_array(0);
  gl.enable_vertex_attrib_array(1);
  gl.vertex_attrib_pointer(0, 4, gl.FLOAT, gl.FALSE, 0, 0)
  gl.vertex_attrib_pointer(1, 4, gl.FLOAT, gl.FALSE, 0, 48)

  gl.draw_arrays(gl.TRIANGLES, 0, 3)

  gl.disable_vertex_attrib_array(0)
  gl.disable_vertex_attrib_array(1)
  gl.use_program(0)

  gl.swap_window()
end
