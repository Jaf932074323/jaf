@echo off
cd /d %~dp0
set curpath=%~dp0
echo ��ʽ������C++����

set varStr=
for /R %curpath%\src %%f in (*.cpp *.c *.h) do call :concat %%f
echo %varStr%
clang-format.exe -i -style=file %varStr%

:: ���� /R ��ʾ��Ҫ�������ļ���,ȥ����ʾ���������ļ���
:: %%f ��һ������,�����ڵ�����,�����������ֻ����һ����ĸ���,ǰ�����%%
:: ��������ͨ���,����ָ����׺��,*.*��ʾ�����ļ�
::for /R %curpath%\src %%f in (*.cpp *.c *.h) do (	
::	clang-format.exe -i -style=file %%f
::)

@pause

::�Բ������м�ǰ׺��ƴ��
:concat
set varStr=%varStr% %1
