NAME=triangle
CXX=c++
CXXFLAGS= -std=c++17 -Ofast -DDEBUG -g -I/home/kali/.libs/stb -I/home/kali/.libs/tinyobjloader
LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

SRC=main.cpp
# SRC=test.cpp
OBJ=$(SRC:.cpp=.o)
SHADERS=shader.frag shader.vert
SPV=frag.spv vert.spv

test: $(NAME)
	./triangle

%.spv: shader.%
	glslc $< -o $@

$(NAME): $(OBJ) $(SPV)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $(NAME) $< -o $@ 

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)

.PHONY: clean fclean re