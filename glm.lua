local Matrix = require 'glm/matrix'

local M = {}

local is_matrix = function(val)
  return getmetatable(val) == Matrix
end

local is_number = function(val)
  return type(val) == "number"
end

function M.mat4(...)
  local args = {...}

  if is_number(args[1]) then
    if #args == 1 then
      return Matrix.new_diagonal(4, args[1])
    elseif #args == 16 then
      return Matrix.new_from_data(4, 4, data)
    end
  else
    error("Invalid arguments to mat4")
  end
end

function M.mat3(value)
  return make_diagonal_matrix(3, value)
end

function M.mat2(value)
  return make_diagonal_matrix(2, value)
end

M.Matrix = Matrix

return M
