@echo off
REM Docker Hub publishing script for MedvedDB development container (Windows)
REM Usage: docker_publish.bat [DOCKER_HUB_USERNAME]

set DOCKER_HUB_USER=%1
if "%DOCKER_HUB_USER%"=="" set DOCKER_HUB_USER=your_dockerhub_username

set IMAGE_NAME=mdv_dev
set TAG_DATE=2025-01-11
set OLD_IMAGE=medveddb-test:latest

echo === MedvedDB Docker Hub Publishing Script ===
echo Tagging existing image: %OLD_IMAGE%
echo New image name: %DOCKER_HUB_USER%/%IMAGE_NAME%:%TAG_DATE%
echo Latest tag: %DOCKER_HUB_USER%/%IMAGE_NAME%:latest

REM Tag the existing image with new name and date
docker tag %OLD_IMAGE% %DOCKER_HUB_USER%/%IMAGE_NAME%:%TAG_DATE%
docker tag %OLD_IMAGE% %DOCKER_HUB_USER%/%IMAGE_NAME%:latest

echo Images tagged successfully!
echo.
echo To publish to Docker Hub, run:
echo   docker login
echo   docker push %DOCKER_HUB_USER%/%IMAGE_NAME%:%TAG_DATE%
echo   docker push %DOCKER_HUB_USER%/%IMAGE_NAME%:latest
echo.
echo To test locally:
echo   docker run --rm -v "%%cd%%":/app -w /app %DOCKER_HUB_USER%/%IMAGE_NAME%:%TAG_DATE% bash -c "rm -rf build && ./run_tests.sh"