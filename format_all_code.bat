@echo off
cd /d %~dp0
set curpath=%~dp0
echo 格式化所有C++代码

set varStr=
for /R %curpath%\src %%f in (*.cpp *.c *.h) do call :concat %%f
echo %varStr%
clang-format.exe -i -style=file %varStr%

:: 参数 /R 表示需要遍历子文件夹,去掉表示不遍历子文件夹
:: %%f 是一个变量,类似于迭代器,但是这个变量只能由一个字母组成,前面带上%%
:: 括号中是通配符,可以指定后缀名,*.*表示所有文件
::for /R %curpath%\src %%f in (*.cpp *.c *.h) do (	
::	clang-format.exe -i -style=file %%f
::)

@pause

::对参数进行加前缀、拼接
:concat
set varStr=%varStr% %1
