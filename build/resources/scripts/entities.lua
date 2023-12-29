function getNew(px, py, w, h)
  local e = { x = px, y = py, width = w, height = h }
  return e
end

return { getNew = getNew} -- local symbols to be exported via require