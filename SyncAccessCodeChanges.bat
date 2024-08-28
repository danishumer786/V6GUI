@ECHO OFF
:: =============================================================================================================
:: 7/21/22 - James Butcher
::
:: This script is a one-click tool for carrying out all the steps needed for deploying any changes
:: that may have been made to the access code files: 
::	"access_codes_factory.txt" and
:: 	"access_codes_service.txt"
::
::   	Step 1 - Update encrypted access code files using python script "access_code_encryptor.py"
::   	Step 2 - Execute Git commands: git add, git commit, git push
:: =============================================================================================================


:: Step 1 -----------------------------

python access_code_encryptor.py


:: Step 2 -----------------------------

::cd Output
::git add fc.txt sc.txt
::git commit -m "sync fc and sc"
::git push
