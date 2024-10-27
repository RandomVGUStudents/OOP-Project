#include "game.hpp"
#include "board.hpp"

Game::Game() : rng(rd())
{
    GenerateBag(currentBag); // Generate this bag
    GenerateBag(currentBag); // Generate next bag

    currentBlock = NextBlock();

    usedHold = false;

    frameCounter = 0;
    lockDownMove = 0;
    lockDownTimer = 0;
    touchedDown = false;
    tSpinDetected = false;
    normalTspin = false;
}

int Game::Update(Action action)
{
    int reward = stats.score;

    if (touchedDown)
    {
        if (CheckValidPos(0, 1))
            lockDownTimer = 0;
        else
        {
            lockDownTimer++;

            if (lockDownMove >= 15 || lockDownTimer > 5)
                LockBlock();
        }
    }

    frameCounter++;
    if (frameCounter >= 5)
    {
        MoveVertical(1);
        frameCounter -= 5;
    }

    switch (action)
    {
        case DO_NOTHING:
            break;

        case HOLD_BLOCK:
            HoldBlock();
            break;

        case SOFT_DROP:
            SoftDrop();
            break;

        case HARD_DROP:
            HardDrop();
            break;

        case MOVE_LEFT:
            MoveHorizontal(true, 1);
            break;

        case MOVE_RIGHT:
            MoveHorizontal(false, 1);
            break;

        case ROTATE_LEFT:
            Rotate(LEFT);
            break;

        case ROTATE_RIGHT:
            Rotate(RIGHT);
            break;

        case ROTATE_DOWN:
            Rotate(DOWN);
            break;
    }

    return stats.score - reward;
}

bool Game::IsGameOver()
{
    bool gameOver = false;
    
    for (size_t i = 0; i < BOARD_WIDTH; ++i)
        if (board.GetCell(i, 2) != EMPTY)
        {
            gameOver = true;
            break;
        }

    return gameOver;
}

void Game::Reset()
{
    currentBag.clear();
    GenerateBag(currentBag); // Generate this bag
    GenerateBag(currentBag); // Generate next bag
    
    board.Reset();

    currentBlock.reset();
    currentBlock = NextBlock();

    usedHold = false;
    holdBlock.reset();

    frameCounter = 0;
    lockDownMove = 0;
    lockDownTimer = 0;
    touchedDown = false;
    tSpinDetected = false;
    normalTspin = false;

    stats = initStats;
}

void Game::Rotate(RotateState direction)
{
    bool rotateCW = (direction == RIGHT);
    bool tBlock = (currentBlock->GetType() == T);
    RotateState currentState = currentBlock->GetRotation();
    RotateState newState = static_cast<RotateState>((currentState + direction) % 4);

    if (direction == DOWN)
    {
        bool success = tBlock
            ? TryRotateOpposite(currentState, newState, TOppositeSrsData)
            : TryRotateOpposite(currentState, newState, OppositeSrsData);

        if (success)
            LockDownReset();
        return;
    }

    size_t testIndex = -1;
    const auto& kickTable = currentBlock->GetType() == I ? IsrsData : srsData;
    for (const Coord& offset : kickTable.at(currentState * 2 + rotateCW).offsets)
    {
        testIndex++;
        if (TryRotateBlock(currentState, newState, offset))
        {
            LockDownReset();
            if (tBlock && touchedDown)
            {
                tSpinDetected = true;
                normalTspin = (testIndex == 4);
            }
            return;
        }
    }
}

void Game::HardDrop()
{
    int droppedLine = GetHardDropPos() - BOARD_HEIGHT + 20;

    stats.score += 2 * droppedLine;
    MoveVertical(droppedLine);
    LockBlock();
}

void Game::SoftDrop()
{
    int droppedLine = GetHardDropPos() - BOARD_HEIGHT + 20;

    stats.score += droppedLine;
    MoveVertical(droppedLine);
}

void Game::HoldBlock()
{
    if (usedHold)
        return;

    if (holdBlock)
        currentBag.push_front(*holdBlock);

    holdBlock.emplace(currentBlock->GetType());
    currentBlock.reset();
    currentBlock = NextBlock();
    usedHold = true;
}

int Game::GetHoles()                                                                                                                                   
{                                                                                                                                                      
    int holes = 0;                                                                                                                                     
    for (size_t i = 0; i < BOARD_WIDTH; ++i)                                                                                                           
    {                                                                                                                                                  
        bool blockFound = false;                                                                                                                       
        for (size_t j = 0; j < BOARD_HEIGHT; ++j)                                                                                                      
        {                                                                                                                                              
            if (board.GetCell(i, j) != EMPTY)                                                                                                          
            {                                                                                                                                          
                blockFound = true;                                                                                                                     
            }                                                                                                                                          
            else if (blockFound)                                                                                                                       
            {                                                                                                                                          
                holes++;                                                                                                                               
            }                                                                                                                                          
        }                                                                                                                                              
    }                                                                                                                                                  
    return holes;                                                                                                                                      
}                                                                                                                                                      

int Game::GetHighestCol()                                                                                                                              
{                                                                                                                                                      
    for (size_t i = 2; i < BOARD_HEIGHT; ++i)                                                                                                          
    {                                                                                                                                                  
        for (size_t j = 0; j < BOARD_WIDTH; ++j)                                                                                                       
        {                                                                                                                                              
            if (board.GetCell(j, i) != EMPTY)                                                                                                          
            {                                                                                                                                          
                return BOARD_HEIGHT - i;                                                                                                               
            }                                                                                                                                          
        }                                                                                                                                              
    }                                                                                                                                                  
    return 0;                                                                                                                                          
} 

void Game::GenerateBag(deque<Block> &bag)
{
    array<Block, BAG_SIZE> newBag = {
        Block(I), Block(J), Block(L), Block(O), Block(S), Block(T), Block(Z)
    };

    shuffle(newBag.begin(), newBag.end(), rng);
    bag.insert(bag.end(), make_move_iterator(newBag.begin()), make_move_iterator(newBag.end()));
}

Block Game::NextBlock()
{
    Block nextBlock = std::move(currentBag.front());
    currentBag.pop_front();

    if (currentBag.size() == BAG_SIZE)
        GenerateBag(currentBag);

    nextBlock.Move(nextBlock.GetType() == O ? 4 : 3, 0);
    return std::move(nextBlock);
}

void Game::LockBlock()
{
    board.LockBlock(*currentBlock);
    currentBlock.reset();
    currentBlock = NextBlock();

    touchedDown = false;
    lockDownMove = 0;
    lockDownTimer = 0;
    usedHold = false;

    stats.droppedBlockCount++;

    if (tSpinDetected)
        ValidateTSpin();

    int clearedLine = board.CheckFullRow();
    if (clearedLine == 0 && !tSpinDetected)
    {
        stats.comboCount = -1;
        return;
    }
    else if (clearedLine)
    {
        stats.comboCount++;
        stats.clearedLineCount += clearedLine;
        stats.score += stats.comboCount * 50;
    }

    int score = 0;

    int holesCount = GetHoles();
    if (holesCount >= stats.holes)
        score -= 50 * (holesCount - stats.holes);
    else
        score += 50 * (stats.holes - holesCount);
    stats.holes = holesCount;

    int highestCol = GetHighestCol();
    if (highestCol > stats.highestCol)
        score -= 10 * (highestCol - stats.highestCol);
    stats.highestCol = highestCol;

    if (tSpinDetected)
    {
        stats.tSpinCount++;
        switch (clearedLine)
        {
            case 0: score = 100 + 300 * normalTspin; break;
            case 1: score = 200 + 600 * normalTspin; stats.b2bChain++; break;
            case 2: score = 400 + 800 * normalTspin; stats.b2bChain++; break;
            case 3: score = 1600; stats.b2bChain++; break;
        }
    } else {
        switch (clearedLine)
        {
            case 1: score = 100; stats.b2bChain = -1; break;
            case 2: score = 300; stats.b2bChain = -1; break;
            case 3: score = 500; stats.b2bChain = -1; break;
            case 4: score = 800; stats.tetrisCount++; stats.b2bChain++; break;
        }
    }

    
    if (stats.b2bChain > 0)
    {
        score *= 1.5;
    }

    if (board.CheckFullClear())
    {
        stats.fullClearCount++;
        switch (clearedLine) {
            case 1: score += 800; break;
            case 2: score += 1200; break;
            case 3: score += 1800; break;
            case 4: score += (stats.b2bChain > 0) ? 3200 : 2000; break;
        }
    }

    stats.score += score;

    tSpinDetected = false;
    normalTspin = false;
}

void Game::LockDownReset()
{
    if (!touchedDown)
        return;

    lockDownMove++;
    lockDownTimer = 0;
}

bool Game::CheckValidPos(int offsetX, int offsetY)
{
    return board.CheckFit(offsetX, offsetY, *currentBlock);
}

void Game::ValidateTSpin()
{
    static const int checkPairs[4][2] = {
        {0, 1},
        {0, 2},
        {2, 3},
        {1, 3}
    };

    Block cornerTester;
    int posX, posY;
    currentBlock->GetPosition(posX, posY);

    bool corners[4] = {
        !board.CheckFit(posX, posY, cornerTester),
        !board.CheckFit(posX + 2, posY, cornerTester),
        !board.CheckFit(posX, posY + 2, cornerTester),
        !board.CheckFit(posX + 2, posY + 2, cornerTester)
    };

    int cornersTouched = corners[0] + corners[1] + corners[2] + corners[3];

    if (cornersTouched <= 2)
    {
        tSpinDetected = false;
        normalTspin = false;
        return;
    }

    if (cornersTouched == 4 || normalTspin)
        return;

    RotateState tState = currentBlock->GetRotation();

    const int* pair = checkPairs[tState];
    if (corners[pair[0]] && corners[pair[1]])
        normalTspin = true;
}

int Game::GetHardDropPos()
{
    int i = 0;
    while (CheckValidPos(0, i++));
    return i;
}

void Game::MoveVertical(int lines)
{
    if (!CheckValidPos(0, lines) || lines == 0)
        return;

    currentBlock->Move(0, lines);

    LockDownReset();
    tSpinDetected = false;
    normalTspin = false;
    if (!CheckValidPos(0, 1))
        touchedDown = true;
}

void Game::MoveHorizontal(bool left, int col)
{
    int steps = left ? -col : col;
    if (!CheckValidPos(steps, 0) || steps == 0)
        return;

    currentBlock->Move(steps, 0);
    LockDownReset();
    tSpinDetected = false;
    normalTspin = false;
}

bool Game::TryRotateBlock(RotateState currentState, RotateState newState, const Coord& offset)
{
    currentBlock->Rotate(newState);
    if (CheckValidPos(offset.x, offset.y))
    {
        currentBlock->Move(offset.x, offset.y);
        return true;
    }
    currentBlock->Rotate(currentState);
    return false;
}

template <size_t N>
bool Game::TryRotateOpposite(RotateState currentState, RotateState newState, 
                             const array<array<Coord, N>, 4>& srsData)
{
    for (const Coord& offset : srsData[currentState])
        if (TryRotateBlock(currentState, newState, offset))
            return true;
    return false;
}
