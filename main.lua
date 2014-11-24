function startup()
  local data = {a=1}
  return data
end

function update_thread_update(data)
  local new_data = {a = data.a + 1}
  return new_data, (data.a >= 10)
end

function render_thread_render(data)
  print(data.a)
end
