CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I./src
LDFLAGS = -lconfig

SRCDIR = src
BUILDDIR = build
TEMPLATEDIR = template

TARGET = txt2epub

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRC))

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Build complete! Executable: $(TARGET)"

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILDDIR) $(TARGET)
	@echo "Clean complete!"

.PHONY: all clean
