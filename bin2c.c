#include <stdio.h>

int main(int argc, char** argv)
{
	if (argc < 4)
		return 1;

	const char* sym = argv[1];

	FILE* in = NULL;
	FILE* out = NULL;

	in = fopen(argv[2], "r");

	if (!in)
	{
		perror("opening input file failed");
		goto fail;
	}

	out = fopen(argv[3], "wt");

	if (!out)
	{
		perror("opening input file failed");
		goto fail;
	}

	if (fprintf(out, "#include <stddef.h>\n") < 0)
	{
		perror("write failed");
		goto fail;
	}

	if (fprintf(out, "const char %s[] = {\n", sym) < 0)
	{
		perror("write failed");
		goto fail;
	}

	unsigned char buf[4096];
	size_t total_size = 0;
	size_t bytes_read;

	while ((bytes_read = fread(buf, 1, sizeof buf, in)) > 0)
	{
		size_t i = 0;

		for (; i + 8 < bytes_read; i += 8)
		{
			int written = fprintf(out,
				"0x%02x, 0x%02x, 0x%02x, 0x%02x, "
				"0x%02x, 0x%02x, 0x%02x, 0x%02x, \n",
				buf[i],   buf[i+1], buf[i+2], buf[i+3],
				buf[i+4], buf[i+5], buf[i+6], buf[i+7]
			);

			if (written < 0)
			{
				perror("write failed");
				goto fail;
			}
		}

		for (; i < bytes_read; ++i)
		{
			int written = fprintf(out, "0x%02x, ", buf[i]);

			if (written < 0)
			{
				perror("write failed");
				goto fail;
			}
		}

		if (fputc('\n', out) == EOF)
		{
			perror("write failed");
			goto fail;
		}

		total_size += bytes_read;
	}

	if (ferror(in))
	{
		perror("read failed");
		goto fail;
	}

	if (fprintf(out, "};\n") < 0)
	{
		perror("write failed");
		goto fail;
	}

	if (fprintf(out, "size_t %s_size = %zu;\n\n", sym, total_size) < 0)
	{
		perror("write failed");
		goto fail;
	}

	return 0;

fail:
	if (in)
		fclose(in);

	if (out)
		fclose(out);

	return 1;
}
