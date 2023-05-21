
build: buildfm buildpreview
buildforce: buildfmforce buildpreviewforce

run:
	"./fm"
	@ echo exited

buildfm: main.c folder.c window.c config.h common.h buildfmforce
buildfmforce:
	cc "main.c" "folder.c" "window.c" -o "fm" -O4

buildpreview: preview.c buildpreviewforce
buildpreviewforce:
	cc "preview.c" -o "preview" -lmagic -O4

	
