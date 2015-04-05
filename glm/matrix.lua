local gl = require 'gl'

local Matrix = {}

local MatrixMetatable = {}
MatrixMetatable.__index = Matrix

function Matrix.new(rows, cols)
  local data = {}

  for i=1,rows * cols do
    data[i] = 0.0
  end

  return Matrix.new_from_data(rows, cols, data)
end

function Matrix.new_from_data(rows, cols, data)
  assert(#data == rows * cols, "Trying to make matrix with the wrong amount of data")

  local mat = {
    data = data,
    rows = rows,
    cols = cols,
  }

  setmetatable(mat, MatrixMetatable)

  return mat
end

function Matrix.new_diagonal(size, value)
  local mat = Matrix.new(size, size)

  for i=1,size do
    mat:set(i, i, value)
  end

  return mat
end

function Matrix:copy()
  local data_copy = {}
  for i=1,self.rows * self.cols do
    data_copy[i] = self.data[i]
  end

  return Matrix.new_from_data(self.rows, self.cols, data_copy)
end

function Matrix:_index(row, col)
  return 1 + (row - 1) + self.rows * (col - 1)
end

function Matrix:at(row, col)
  return self.data[self:_index(row, col)]
end

function Matrix:set(row, col, value)
  self.data[self:_index(row, col)] = value
end

function Matrix:row(row)
  local data = {}

  for i=1,self.cols do
    data[i] = self:at(row, i)
  end

  return data
end

function Matrix:print()
  for i=1,self.rows do
    print(unpack(self:row(i)))
  end
end

function Matrix:to_uniform(uniform)
  gl.uniform_matrix_float(
    -- TODO(emily): Make sure rows/cols aren't flipped
    uniform, self.rows, self.cols,
    self.data
  )
end

function Matrix.multiply(a, b)
  assert(getmetatable(a) == MatrixMetatable, "Can't Matrix.multiply non-matrices")
  assert(getmetatable(b) == MatrixMetatable, "Can't Matrix.multiply non-matrices")

  assert(a.cols == b.rows, "Invalid dimensions for Matrix.multiply")

  local new_rows = a.rows
  local new_cols = b.cols

  local mat = Matrix.new(new_rows, new_cols)

  for i=1,new_rows do
    for j=1,new_cols do
      for k=1,a.cols do
        mat:set(
          i, j,
          mat:at(i, j) + a:at(i, k) * b:at(k, j)
        )
      end
    end
  end

  return mat
end
MatrixMetatable.__mul = Matrix.multiply

function Matrix.new_translation(x, y, z)
  return Matrix.new_from_data(
    4, 4,
    {
      1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0,
      x,   y,   z,   1.0,
    }
  )
end

function Matrix:translate(x, y, z)
  return self * Matrix.new_translation(x, y, z)
end

function Matrix.new_scale(x, y, z)
  return Matrix.new_from_data(
    4, 4,
    {
      x,   0.0, 0.0, 0.0,
      0.0, y,   0.0, 0.0,
      0.0, 0.0, z,   0.0,
      0.0, 0.0, 0.0, 1.0,
    }
  )
end

function Matrix:scale(x, y, z)
  return self * Matrix.new_scale(x, y, z)
end

function Matrix.new_rotation(x, y, z, angle)
  local c = math.cos(angle)
  local ic = 1 - c

  local s = math.sin(angle)
  local is = 1 - s

  return Matrix.new_from_data(
    4, 4,
    {
      x * x + (1 - x * x) * c, ic * x * y + z * s,      ic * x * y - y * s,      0.0,
      ic * x * y - z * s,      y * y + (1 - y * y) * c, ic * y * z + x * s,      0.0,
      ic * x * z + y * s,      ic * y * z - x * s,      z * z + (1 - z * z) * c, 0.0,
      0.0,                     0.0,                     0.0,                     1.0,
    }
  )
end

function Matrix:rotate(x, y, z, angle)
  return self * Matrix.new_rotation(x, y, z, angle)
end

return Matrix
