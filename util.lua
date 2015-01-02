local M = {}

function M.extend(...)
  obj = nil

  for _, arg in ipairs{...} do
    if obj == nil then
      obj = arg
    else
      for k, v in pairs(arg) do
        obj[k] = v
      end
    end
  end

  return obj
end

return M
