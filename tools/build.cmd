@for %%f in (*.sln) do (
  setlocal
  call cmd\build_vc %%~nf %*
  endlocal
)
