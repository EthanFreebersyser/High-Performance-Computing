// png_dir_to_video.c
// Build (Linux/macOS): gcc -O2 -Wall -Wextra -o png_dir_to_video png_dir_to_video.c
// Usage:
//   ./png_dir_to_video <frames_dir> <output.mp4> [fps]
// Example:
//   ./png_dir_to_video ./frames out.mp4 60
//
// Requirements:
//   - ffmpeg installed and in PATH
//   - frames named 0.png, 1.png, 2.png, ... with no gaps

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int ends_with(const char *s, const char *suffix) {
    size_t ls = strlen(s), lsf = strlen(suffix);
    return (ls >= lsf) && (strcmp(s + (ls - lsf), suffix) == 0);
}

static int is_digits_only(const char *s) {
    if (!s || !*s) return 0;
    for (const unsigned char *p = (const unsigned char *)s; *p; ++p) {
        if (!isdigit(*p)) return 0;
    }
    return 1;
}

static int count_png_frames(const char *dirpath) {
    DIR *d = opendir(dirpath);
    if (!d) {
        fprintf(stderr, "Error: cannot open dir '%s': %s\n", dirpath, strerror(errno));
        return -1;
    }

    int count = 0;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        const char *name = ent->d_name;

        // We only count files that look like "<number>.png" (e.g., 0.png, 12.png)
        if (!ends_with(name, ".png")) continue;

        char base[NAME_MAX + 1];
        strncpy(base, name, sizeof(base) - 1);
        base[sizeof(base) - 1] = '\0';

        // Strip ".png"
        char *dot = strrchr(base, '.');
        if (!dot) continue;
        *dot = '\0';

        if (is_digits_only(base)) count++;
    }

    closedir(d);
    return count;
}

int main(int argc, char **argv) {
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s <frames_dir> <output.mp4> [fps]\n", argv[0]);
        return 2;
    }

    const char *frames_dir = argv[1];
    const char *out_file   = argv[2];
    int fps = 30;

    if (argc == 4) {
        fps = atoi(argv[3]);
        if (fps <= 0 || fps > 240) {
            fprintf(stderr, "Error: fps must be in 1..240\n");
            return 2;
        }
    }

    int n = count_png_frames(frames_dir);
    if (n < 0) return 1;

    if (n == 0) {
        fprintf(stderr, "Error: found 0 frames in '%s'\n", frames_dir);
        return 1;
    }

    // We assume frames are 0..(n-1). If you might have gaps, you should validate separately.
    // Build: ffmpeg -y -framerate <fps> -i "<dir>/%d.png" -c:v libx264 -pix_fmt yuv420p -crf 18 -preset medium "<out>"
    char cmd[4096];
    int written = snprintf(
        cmd, sizeof(cmd),
        "ffmpeg -y -hide_banner -loglevel error "
        "-framerate %d "
        "-i \"%s/%%d.png\" "
        "-c:v libx264 -pix_fmt yuv420p -crf 18 -preset medium "
        "\"%s\"",
        fps, frames_dir, out_file
    );

    if (written < 0 || (size_t)written >= sizeof(cmd)) {
        fprintf(stderr, "Error: command too long\n");
        return 1;
    }

    printf("Frames detected: %d\n", n);
    printf("Running: %s\n", cmd);

    int rc = system(cmd);
    if (rc != 0) {
        fprintf(stderr, "Error: ffmpeg failed (exit code %d)\n", rc);
        return 1;
    }

    printf("Done: %s\n", out_file);
    return 0;
}
