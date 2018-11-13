for %%i in (*.proto) do (    
echo %%i  
".\protoc.exe" --descriptor_set_out=./pb/%%i ./%%i
  
)  
pause 