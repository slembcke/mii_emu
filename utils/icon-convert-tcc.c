/*
	first convert the .png to binary files in contrib using imagemagik:
   $ convert your_image.png -depth 8 rgba:your_image.bin
	run this with
   $ tcc -run icon-convert-tcc.c
 */
#include <glob.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <libgen.h>

/* this loads each contrib/mii-icon-*.bin file which is raw RGBA and
 add the header width/height, and convert the pixels to ARGB */

int main() {
	glob_t g = {};
	glob("contrib/mii-icon-*.bin", 0, NULL, &g);
	for (int i = 0; i < g.gl_pathc; i++) {
		char * path = strdup(g.gl_pathv[i]);
		char * name = basename(path);
		char * dot = strrchr(name, '.');
		if (dot)
			*dot = 0;
		char * dup = strdup(name);
		char * size = strrchr(dup, '-');
		if (size)
			size++;
		int w = 0, h = 0;
		if (size)
			w = h = atoi(size);
		char *orig = g.gl_pathv[i];
		if (!w || !h) {
			printf("bad size %s\n", orig);
			continue;
		}
		FILE * f = fopen(orig, "rb");
		if (!f) {
			perror(orig);
			printf("bad file %s\n", orig);
			continue;
		}
		fseek(f, 0, SEEK_END);
		int size_ = ftell(f);
		fseek(f, 0, SEEK_SET);
		unsigned char * pixels = malloc(size_);
		fread(pixels, 1, size_, f);
		fclose(f);
		char out[1024];
		snprintf(out, sizeof(out), "contrib/%s.h", name);
		f = fopen(out, "wb");
		if (!f) {
			printf("bad file %s\n", out);
			continue;
		}
		fprintf(f, "/* this file is auto-generated by icon-convert-tcc.c */\n");
	//	fprintf(f, "#include <stdint.h>\n");
		fprintf(f, "#define MII_ICON%d_SIZE %d\n", w, (w * h + 2));
		fprintf(f, "extern const unsigned long mii_icon%d[MII_ICON%d_SIZE];\n", w, w);
		fprintf(f, "#ifdef MII_ICON%d_DEFINE\n", w);
		fprintf(f, "const unsigned long mii_icon%d[MII_ICON%d_SIZE] = {\n",
					w, w);
		fprintf(f, "\t%d,%d,\n", w, h);
		for (int y = 0; y < h; y++) {
			fprintf(f, "\t");
			for (int x = 0; x < w; x++) {
				int i = (y * w + x) * 4;
				int r = pixels[i + 0];
				int g = pixels[i + 1];
				int b = pixels[i + 2];
				int a = pixels[i + 3];
				fprintf(f, "0x%02x%02x%02x%02x, ", a, r, g, b);
				if (x % 8 == 7)
					fprintf(f, "\n\t");
			}
			fprintf(f, "\n");
		}
		fprintf(f, "};\n");
		fprintf(f, "#endif\n");
		fclose(f);
		free(pixels);
	}
}