NAME=logger
CXX=c++
CXXFLAGS= -std=c++17 -Ofast -DDEBUG -g #-I$(LIBS)/stb

SRC_DIR =src/
SRC_FILE=$(wildcard $(SRC_DIR)*.cpp)
SRC=$(addprefix $(SRC_DIR), $(SRC_FILE))
OBJ_DIR = obj/
OBJ=$(addprefix $(OBJ_DIR), $(notdir $(SRC:.cpp=.o)))

test: $(NAME)
	./logger

$(NAME): $(OBJ_DIR) $(OBJ) $(SPV)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJ)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp $(SRC_DIR)%.hpp
	$(CXX) $(CXXFLAGS) -c -o $(NAME) $< -o $@ 

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp
	$(CXX) $(CXXFLAGS) -c -o $(NAME) $< -o $@ 

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)

.PHONY: clean fclean re