FROM mcr.microsoft.com/windows:ltsc2019 AS base

WORKDIR /tmp

RUN powershell (New-Object System.Net.WebClient).DownloadFile('https://repo.continuum.io/miniconda/Miniconda3-latest-Windows-x86_64.exe', 'Miniconda3.exe')
RUN powershell Unblock-File Miniconda3.exe
RUN Miniconda3.exe /InstallationType=JustMe /RegisterPython=1 /S /D=C:\Miniconda3
FROM microsoft/nanoserver:latest
COPY --from=base C:\Miniconda3 C:\Miniconda3
RUN setx /M PATH "%PATH%;C:\Miniconda3;C:\Miniconda3\Scripts;C:\Miniconda3\Library\bin"

WORKDIR /modelserver
COPY . /modelserver
RUN ["conda", "create", "-n", "pytorch-env", "--file", "requirements.txt"]
RUN conda init powershell
SHELL ["powershell", "-Command", "conda activate myenv"]
EXPOSE 8822
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8822"]