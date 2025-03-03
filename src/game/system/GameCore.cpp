//#include "GameCore.hpp"
//#include "../../texture/ImageTexture.hpp"
//#include "../block/IceBlock.hpp"
//#include "../../core/GameApp.hpp"
//#include "../../utils/Logger.hpp"
//
//#include <random>
//#include <algorithm>
//#include <cstring>
//
//GameCore::GameCore()
//    : boardX_(0.0f)
//    , boardY_(0.0f)
//{
//}
//
//GameCore::~GameCore() = default;
//
//bool GameCore::Initialize(float boardX, float boardY)
//{
//    boardX_ = boardX;
//    boardY_ = boardY;
//
//    // ���� ���� �ʱ�ȭ
//    stateInfo_ = GameStateInfo{};
//    stateInfo_.currentPhase = GamePhase::Playing;
//    stateInfo_.previousPhase = GamePhase::Playing;
//
//    // ���� ���� �ʱ�ȭ
//    scoreInfo_ = ScoreInfo{};
//
//    // ���� �ʱ�ȭ
//    ClearBoard();
//
//    return true;
//}
//
//void GameCore::Reset()
//{
//    // ���� ���� �ʱ�ȭ
//    stateInfo_ = GameStateInfo{};
//    stateInfo_.currentPhase = GamePhase::Playing;
//    stateInfo_.previousPhase = GamePhase::Playing;
//
//    // ���� ���� �ʱ�ȭ
//    scoreInfo_.reset();
//}
//
//void GameCore::Update(float deltaTime)
//{
//    // ���� �ð� ������Ʈ
//    stateInfo_.playTime += deltaTime;
//}
//
//void GameCore::SetGamePhase(GamePhase newPhase)
//{
//    if (stateInfo_.currentPhase == newPhase) {
//        return;
//    }
//
//    stateInfo_.previousPhase = stateInfo_.currentPhase;
//    stateInfo_.currentPhase = newPhase;
//
//    // �̺�Ʈ �˸�
//    NotifyEvent(GameCoreEvent::PhaseChanged);
//}
//
//void GameCore::SetEventListener(EventCallback callback)
//{
//    eventCallback_ = callback;
//}
//
//void GameCore::NotifyEvent(GameCoreEvent event, const void* data)
//{
//    if (eventCallback_) {
//        eventCallback_(event, data);
//    }
//}
//
//void GameCore::AddInterruptBlockCount(int16_t count)
//{
//    scoreInfo_.totalInterruptBlockCount += count;
//    NotifyEvent(GameCoreEvent::InterruptCountChanged);
//}
//
//// GameCore.cpp ���
//
//bool GameCore::FindMatchedBlocks(std::vector<std::vector<Block*>>& matchedGroups)
//{
//    std::vector<Block*> currentGroup;
//
//    // ���� ���� ��ü�� ��ȸ�ϸ鼭 ��ġ�� ��� �׷� ã��
//    for (int y = 0; y < Constants::Board::BOARD_Y_COUNT; y++) 
//    {
//        for (int x = 0; x < Constants::Board::BOARD_X_COUNT; x++) 
//        {
//            Block* block = boardBlocks_[y][x];
//
//            if (!block || block->GetBlockType() == BlockType::Ice || block->IsRecursionCheck()) 
//            {
//                continue;
//            }
//
//            block->SetRecursionCheck(true);
//            currentGroup.clear();
//
//            // ��������� ��ġ�Ǵ� ��� ã��
//            if (RecursionCheckBlock(x, y, Constants::Direction::None, currentGroup) >= Constants::Game::MIN_MATCH_COUNT - 1) 
//            {
//                currentGroup.push_back(block);
//                matchedGroups.push_back(currentGroup);
//            }
//            else 
//            {
//                block->SetRecursionCheck(false);
//
//                for (auto* matchedBlock : currentGroup) 
//                {
//                    matchedBlock->SetRecursionCheck(false);
//                }
//            }
//        }
//    }
//
//    return !matchedGroups.empty();
//}
//
//short GameCore::RecursionCheckBlock(short x, short y, Constants::Direction direction,
//    std::vector<Block*>& matchedBlocks)
//{
//    if (!boardBlocks_[y][x]) 
//    {
//        return 0;
//    }
//
//    Block* block = boardBlocks_[y][x];
//    BlockType blockType = block->GetBlockType();
//    short matchCount = 0;
//
//    // �� ���� �˻�
//    const std::array<std::pair<Constants::Direction, std::pair<short, short>>, 4> directions = {
//        {
//            {Constants::Direction::Left,   {x - 1, y}},
//            {Constants::Direction::Right,  {x + 1, y}},
//            {Constants::Direction::Top,    {x, y + 1}},
//            {Constants::Direction::Bottom, {x, y - 1}}
//        }
//    };
//
//    for (const auto& [dir, pos] : directions) 
//    {
//        if (direction == dir) 
//        {
//            continue;
//        }
//
//        const auto [checkX, checkY] = pos;
//        if (checkX < 0 || checkX >= Constants::Board::BOARD_X_COUNT ||
//            checkY < 0 || checkY >= Constants::Board::BOARD_Y_COUNT) 
//        {
//            continue;
//        }
//
//        Block* checkBlock = boardBlocks_[checkY][checkX];
//        if (!checkBlock || checkBlock->IsRecursionCheck() || checkBlock->GetState() != BlockState::Stationary) 
//        {
//            continue;
//        }
//
//        if (blockType == checkBlock->GetBlockType()) 
//        {
//            checkBlock->SetRecursionCheck(true);
//            matchedBlocks.push_back(checkBlock);
//            matchCount++;
//
//            // �ݴ� �������� ��� �˻� (���� �������� �ߺ� Ž�� ����)
//            auto oppositeDir = [](Constants::Direction d) -> Constants::Direction 
//                {
//                    switch (d) 
//                    {
//                    case Constants::Direction::Left: return Constants::Direction::Right;
//                    case Constants::Direction::Right: return Constants::Direction::Left;
//                    case Constants::Direction::Top: return Constants::Direction::Bottom;
//                    case Constants::Direction::Bottom: return Constants::Direction::Top;
//                    default: return Constants::Direction::None;
//                    }
//                };
//
//            matchCount += RecursionCheckBlock(checkX, checkY, oppositeDir(dir), matchedBlocks);
//        }
//    }
//
//    return matchCount;
//}
//
//void GameCore::UpdateLinkState(Block* block)
//{
//    if (!block || block->GetBlockType() == BlockType::Ice) 
//    {
//        return;
//    }
//
//    int x = block->GetPosIdx_X();
//    int y = block->GetPosIdx_Y();
//    BlockType blockType = block->GetBlockType();
//    uint8_t linkState = static_cast<uint8_t>(block->GetLinkState());
//
//    // �� ���⺰ ���� ���� �˻�
//    const std::array<std::pair<Constants::Direction, std::pair<int, int>>, 4> directions = 
//    {
//        {
//            {Constants::Direction::Left,   {x - 1, y}},
//            {Constants::Direction::Right,  {x + 1, y}},
//            {Constants::Direction::Top,    {x, y + 1}},
//            {Constants::Direction::Bottom, {x, y - 1}}
//        }
//    };
//
//    for (const auto& [dir, pos] : directions) 
//    {
//        const auto [checkX, checkY] = pos;
//
//        if (checkX >= 0 && checkX < Constants::Board::BOARD_X_COUNT &&
//            checkY >= 0 && checkY < Constants::Board::BOARD_Y_COUNT) 
//        {
//            Block* checkBlock = boardBlocks_[checkY][checkX];
//
//            if (checkBlock &&
//                checkBlock->GetState() == BlockState::Stationary &&
//                checkBlock->GetBlockType() == blockType) 
//            {
//
//                // ���� ���� ������Ʈ
//                auto checkBlockLinkState = static_cast<uint8_t>(checkBlock->GetLinkState());
//
//                switch (dir) 
//                {
//                case Constants::Direction::Left:
//                    linkState |= static_cast<uint8_t>(LinkState::Right);
//                    checkBlock->SetLinkState(static_cast<LinkState>(checkBlockLinkState | static_cast<uint8_t>(LinkState::Left)));
//                    break;
//
//                case Constants::Direction::Right:
//                    linkState |= static_cast<uint8_t>(LinkState::Left);
//                    checkBlock->SetLinkState(static_cast<LinkState>(checkBlockLinkState | static_cast<uint8_t>(LinkState::Right)));
//                    break;
//
//                case Constants::Direction::Top:
//                    linkState |= static_cast<uint8_t>(LinkState::Bottom);
//                    checkBlock->SetLinkState(static_cast<LinkState>(checkBlockLinkState | static_cast<uint8_t>(LinkState::Top)));
//                    break;
//
//                case Constants::Direction::Bottom:
//                    linkState |= static_cast<uint8_t>(LinkState::Top);
//                    checkBlock->SetLinkState(static_cast<LinkState>(checkBlockLinkState | static_cast<uint8_t>(LinkState::Bottom)));
//                    break;
//                }
//            }
//        }
//    }
//
//    block->SetLinkState(static_cast<LinkState>(linkState));
//}
//
//// ���� ��� ���� �޼���
//void GameCore::CalculateScore(const std::vector<std::vector<Block*>>& matchedGroups)
//{
//    // �޺� ���� ������Ʈ
//    if (stateInfo_.previousPhase == GamePhase::Shattering) {
//        scoreInfo_.comboCount++;
//    }
//    else if (stateInfo_.previousPhase == GamePhase::Playing) {
//        scoreInfo_.comboCount = 1;
//    }
//
//    uint8_t linkBonus = 0;
//    uint8_t blockCount = 0;
//    uint8_t typeBonus = GetTypeBonus(matchedGroups.size());
//    short comboBonus = GetComboConstant(scoreInfo_.comboCount);
//
//    // ��ġ�� ��ϵ鿡 ���� ���ʽ� ���
//    for (const auto& group : matchedGroups) 
//    {
//        for (auto* block : group) 
//        {
//            block->SetState(BlockState::Destroying);
//        }
//
//        linkBonus += GetLinkBonus(group.size());
//        blockCount += static_cast<uint8_t>(group.size());
//    }
//
//    // ���� ���� ���
//    int currentScore = ((blockCount * Constants::Game::Score::BASE_MATCH_SCORE) * (comboBonus + linkBonus + typeBonus + 1));
//
//    // ���� ��� ī��Ʈ ���
//    scoreInfo_.addInterruptBlockCount = (currentScore + scoreInfo_.restScore) / GetMargin();
//    scoreInfo_.restScore = (currentScore + scoreInfo_.restScore) % GetMargin();
//    scoreInfo_.totalScore += currentScore;
//
//    // ���� ��� ���� ������Ʈ
//    UpdateInterruptBlockState();
//
//    // �̺�Ʈ �˸�
//    NotifyEvent(GameCoreEvent::ScoreChanged);
//    NotifyEvent(GameCoreEvent::ComboChanged);
//}
//
//short GameCore::GetComboConstant(uint8_t comboCount) const
//{
//    if (comboCount <= 1) {
//        return 0;
//    }
//
//    if (comboCount <= 4) {
//        return static_cast<short>(std::pow(2, comboCount + 1));
//    }
//
//    if (comboCount <= Constants::Game::MAX_COMBO) {
//        return 32 * (comboCount - 3);
//    }
//
//    return 0;
//}
//
//uint8_t GameCore::GetLinkBonus(size_t linkCount) const
//{
//    static const std::array<uint8_t, 8> LINK_BONUSES = { 0, 0, 0, 0, 2, 3, 4, 5 };
//
//    if (linkCount <= 4) {
//        return 0;
//    }
//    else if (linkCount <= 10) {
//        return LINK_BONUSES[linkCount - 4];
//    }
//    else {
//        return Constants::Game::Score::MAX_LINK_BONUS;
//    }
//}
//
//uint8_t GameCore::GetTypeBonus(size_t count) const
//{
//    static const std::array<uint8_t, 6> TYPE_BONUSES = {
//        0, 0, 3, 6, 12, Constants::Game::Score::MAX_TYPE_BONUS
//    };
//
//    return count < TYPE_BONUSES.size() ? TYPE_BONUSES[count] : TYPE_BONUSES.back();
//}
//
//uint8_t GameCore::GetMargin() const
//{
//    const float playTime = stateInfo_.playTime;
//
//    // �ð��� ���� ���� �� ���
//    for (const auto& margin : Constants::Game::SCORE_MARGINS) {
//        if (playTime <= margin.time) {
//            return margin.margin;
//        }
//    }
//
//    return Constants::Game::SCORE_MARGINS[std::size(Constants::Game::SCORE_MARGINS) - 1].margin;
//}
//
//void GameCore::UpdateComboState()
//{
//    if (stateInfo_.previousPhase == GamePhase::Shattering) {
//        scoreInfo_.comboCount++;
//        NotifyEvent(GameCoreEvent::ComboChanged);
//    }
//    else if (stateInfo_.previousPhase == GamePhase::Playing) {
//        scoreInfo_.comboCount = 1;
//        NotifyEvent(GameCoreEvent::ComboChanged);
//    }
//}
//
//void GameCore::ResetComboState()
//{
//    if (scoreInfo_.comboCount > 0) {
//        scoreInfo_.comboCount = 0;
//        NotifyEvent(GameCoreEvent::ComboChanged);
//    }
//
//    if (scoreInfo_.restScore > 0) {
//        scoreInfo_.restScore = 0;
//    }
//}
//
//
//void GameCore::UpdateInterruptBlockState()
//{
//    bool oldHasIce = stateInfo_.hasIceBlock;
//
//    if (scoreInfo_.totalInterruptBlockCount > 0) {
//        scoreInfo_.totalInterruptBlockCount -= scoreInfo_.addInterruptBlockCount;
//
//        if (scoreInfo_.totalInterruptBlockCount < 0) {
//            scoreInfo_.addInterruptBlockCount = std::abs(scoreInfo_.totalInterruptBlockCount);
//            stateInfo_.hasIceBlock = false;
//            scoreInfo_.totalInterruptBlockCount = 0;
//
//            scoreInfo_.totalEnemyInterruptBlockCount += scoreInfo_.addInterruptBlockCount;
//        }
//        else {
//            stateInfo_.hasIceBlock = true;
//        }
//    }
//    else {
//        if (scoreInfo_.addInterruptBlockCount > 0) {
//            scoreInfo_.totalEnemyInterruptBlockCount += scoreInfo_.addInterruptBlockCount;
//        }
//
//        stateInfo_.hasIceBlock = false;
//        scoreInfo_.totalInterruptBlockCount = 0;
//    }
//
//    // ���°� ����Ǿ����� �̺�Ʈ �߻�
//    if (oldHasIce != stateInfo_.hasIceBlock || scoreInfo_.addInterruptBlockCount > 0) {
//        NotifyEvent(GameCoreEvent::InterruptCountChanged);
//    }
//}
//
//void GameCore::GenerateIceBlocks(std::list<std::shared_ptr<Block>>& blockList,
//    std::set<std::shared_ptr<IceBlock>>& iceBlocks,
//    uint8_t playerID)
//{
//    if (scoreInfo_.totalInterruptBlockCount <= 0 ||
//        stateInfo_.currentPhase != GamePhase::Playing) {
//        return;
//    }
//
//    auto texture = ImageTexture::Create("PUYO/puyo_beta.png");
//    if (!texture) {
//        LOGGER.Error("Failed to get ice block texture");
//        return;
//    }
//
//    // ��� ���� ���� ���� ��� ����
//    if (scoreInfo_.totalInterruptBlockCount > 30) {
//        GenerateLargeIceBlockGroup(texture, blockList, iceBlocks, playerID);
//    }
//    else {
//        GenerateSmallIceBlockGroup(texture, blockList, iceBlocks, playerID);
//    }
//
//    // �̺�Ʈ �߻�
//    NotifyEvent(GameCoreEvent::InterruptCountChanged);
//
//    // ������ ��ȯ
//    SetGamePhase(GamePhase::IceBlocking);
//}
//
//void GameCore::GenerateLargeIceBlockGroup(const std::shared_ptr<ImageTexture>& texture,
//    std::list<std::shared_ptr<Block>>& blockList,
//    std::set<std::shared_ptr<IceBlock>>& iceBlocks,
//    uint8_t playerID)
//{
//    scoreInfo_.totalInterruptBlockCount -= 30;
//
//    // 5x6 ũ���� ���� ��� �׷� ����
//    for (int y = 0; y < 5; y++) {
//        for (int x = 0; x < Constants::Board::BOARD_X_COUNT; x++) {
//            auto iceBlock = std::make_shared<IceBlock>();
//            InitializeIceBlock(iceBlock.get(), texture, x, y, playerID);
//            blockList.push_back(iceBlock);
//            iceBlocks.insert(iceBlock);
//
//            // ���� ���忡 ��� ���
//            boardBlocks_[y][x] = iceBlock.get();
//        }
//    }
//}
//
//void GameCore::GenerateSmallIceBlockGroup(const std::shared_ptr<ImageTexture>& texture,
//    std::list<std::shared_ptr<Block>>& blockList,
//    std::set<std::shared_ptr<IceBlock>>& iceBlocks,
//    uint8_t playerID)
//{
//    const auto yCnt = scoreInfo_.totalInterruptBlockCount / Constants::Board::BOARD_X_COUNT;
//    const auto xCnt = scoreInfo_.totalInterruptBlockCount % Constants::Board::BOARD_X_COUNT;
//
//    // ��ü �� ����
//    for (int y = 0; y < yCnt; y++) {
//        for (int x = 0; x < Constants::Board::BOARD_X_COUNT; x++) {
//            auto iceBlock = std::make_shared<IceBlock>();
//            InitializeIceBlock(iceBlock.get(), texture, x, y, playerID);
//            blockList.push_back(iceBlock);
//            iceBlocks.insert(iceBlock);
//
//            // ���� ���忡 ��� ���
//            boardBlocks_[y][x] = iceBlock.get();
//        }
//    }
//
//    // ���� ��� ���� ��ġ
//    if (xCnt > 0) {
//        std::random_device rd;
//        std::mt19937 gen(rd());
//        std::uniform_int_distribution<> dist(0, Constants::Board::BOARD_X_COUNT - 1);
//
//        std::set<int> positions;
//        while (positions.size() < xCnt) {
//            positions.insert(dist(gen));
//        }
//
//        for (int pos : positions) {
//            auto iceBlock = std::make_shared<IceBlock>();
//            InitializeIceBlock(iceBlock.get(), texture, pos, yCnt, playerID);
//            blockList.push_back(iceBlock);
//            iceBlocks.insert(iceBlock);
//
//            // ���� ���忡 ��� ���
//            boardBlocks_[yCnt][pos] = iceBlock.get();
//        }
//    }
//
//    scoreInfo_.totalInterruptBlockCount = 0;
//}
//
//void GameCore::InitializeIceBlock(IceBlock* block, const std::shared_ptr<ImageTexture>& texture,
//    int x, int y, uint8_t playerID)
//{
//    if (!block) {
//        return;
//    }
//
//    // ��� �⺻ �Ӽ� ����
//    block->SetBlockType(BlockType::Ice);
//    block->SetLinkState(LinkState::Max);
//    block->SetState(BlockState::DownMoving);
//    block->SetBlockTex(texture);
//
//    // �ε��� ��ġ ����
//    block->SetPosIdx(x, y);
//
//    // ���� ������ ��ġ ���
//    float renderX = Constants::Board::WIDTH_MARGIN + Constants::Block::SIZE * x;
//    float renderY = -Constants::Block::SIZE * (y + 1);  // ���������� ����
//    block->SetPosition(renderX, renderY);
//
//    // ũ�� ����
//    block->SetScale(Constants::Block::SIZE, Constants::Block::SIZE);
//
//    // �÷��̾� ID ����
//    block->SetPlayerID(playerID);
//}
//
//
//void GameCore::CollectRemoveIceBlocks(const std::vector<std::vector<Block*>>& matchedGroups,
//    std::set<std::shared_ptr<IceBlock>>& iceBlocks)
//{
//    if (matchedGroups.empty() || stateInfo_.currentPhase != GamePhase::Shattering) {
//        return;
//    }
//
//    // ��ġ�� ��� �ֺ��� ���� ��� ����
//    for (const auto& group : matchedGroups) {
//        for (auto* block : group) {
//            const int x = block->GetPosIdx_X();
//            const int y = block->GetPosIdx_Y();
//
//            // �ֺ� 4���� �˻�
//            const std::array<std::pair<int, int>, 4> directions = {
//                {
//                    {x - 1, y}, {x + 1, y}, {x, y - 1}, {x, y + 1}
//                }
//            };
//
//            for (const auto& [checkX, checkY] : directions) 
//            {
//                if (checkX >= 0 && checkX < Constants::Board::BOARD_X_COUNT &&
//                    checkY >= 0 && checkY < Constants::Board::BOARD_Y_COUNT) 
//                {
//
//                    if (auto* checkBlock = boardBlocks_[checkY][checkX]) 
//                    {
//                        if (checkBlock->GetBlockType() == BlockType::Ice &&
//                            checkBlock->GetState() == BlockState::Stationary) 
//                        {
//
//                            if (auto iceBlock = dynamic_cast<IceBlock*>(checkBlock)) 
//                            {
//                                iceBlock->SetState(BlockState::Destroying);
//
//                                // �̹� �ִ� ������� Ȯ��
//                                auto blockPtr = std::find_if(iceBlocks.begin(), iceBlocks.end(),
//                                    [iceBlock](const std::shared_ptr<IceBlock>& existing) 
//                                    {
//                                        return existing.get() == iceBlock;
//                                    });
//
//                                // ������ ���� �߰�
//                                if (blockPtr == iceBlocks.end()) {
//                                    // ���⼭�� ���� �����͸� ã�Ƽ� �߰��ؾ� �մϴ�
//                                    // ���� �ڵ忡���� ������ ������� ó���ؾ� �մϴ�
//                                }
//                            }
//                        }
//                    }
//                }
//            }
//        }
//    }
//}
//
//
//bool GameCore::IsGameOver()
//{
//    // ��� �߾� 4ĭ�� ä���� �ִ��� Ȯ��
//    if (boardBlocks_[Constants::Board::BOARD_Y_COUNT - 1][2] != nullptr ||
//        boardBlocks_[Constants::Board::BOARD_Y_COUNT - 1][3] != nullptr ||
//        boardBlocks_[Constants::Board::BOARD_Y_COUNT - 2][2] != nullptr ||
//        boardBlocks_[Constants::Board::BOARD_Y_COUNT - 2][3] != nullptr) {
//
//        // ���� ���� �̺�Ʈ �߻�
//        NotifyEvent(GameCoreEvent::GameOver);
//
//        // ���� ���� ����
//        stateInfo_.currentPhase = GamePhase::Standing;
//        return true;
//    }
//
//    return false;
//}
//
//void GameCore::UpdateBlockPositions()
//{
//    // �� ������ ó��
//    for (int x = 0; x < Constants::Board::BOARD_X_COUNT; x++) {
//        int emptySpaces = 0;
//
//        // �Ʒ��������� ���� �˻�
//        for (int y = 0; y < Constants::Board::BOARD_Y_COUNT; y++) {
//            if (boardBlocks_[y][x] == nullptr) {
//                emptySpaces++;
//            }
//            else if (emptySpaces > 0) {
//                // �� ������ ������ ����� �Ʒ��� �̵�
//                Block* block = boardBlocks_[y][x];
//                boardBlocks_[y][x] = nullptr;
//                boardBlocks_[y - emptySpaces][x] = block;
//
//                // ����� �ε����� ���� ������Ʈ
//                block->SetPosIdx(x, y - emptySpaces);
//                block->SetState(BlockState::DownMoving);
//            }
//        }
//    }
//}
//
//bool GameCore::CheckGameBlockState(std::list<std::shared_ptr<Block>>& blockList)
//{
//    // ��� ���� Ȯ��
//    int blockCount = static_cast<int>(blockList.size());
//
//    // ���� ���� ���� Ȯ��
//    if (stateInfo_.shouldQuit) {
//        NotifyEvent(GameCoreEvent::GameOver);
//        stateInfo_.currentPhase = GamePhase::Standing;
//        return true;
//    }
//
//    // �ּ� ��� �� Ȯ��
//    if (blockCount < Constants::Game::MIN_MATCH_COUNT) {
//        stateInfo_.currentPhase = GamePhase::Playing;
//        stateInfo_.previousPhase = GamePhase::Playing;
//        ResetComboState();
//        return false;
//    }
//
//    // ��ġ�� ��� ã��
//    std::vector<std::vector<Block*>> matchedGroups;
//    if (FindMatchedBlocks(matchedGroups)) {
//        stateInfo_.previousPhase = stateInfo_.currentPhase;
//        SetGamePhase(GamePhase::Shattering);
//
//        // ��ġ�� ����� ���� ���
//        CalculateScore(matchedGroups);
//
//        // ���� ��� ���� �� ����
//        if (!stateInfo_.isDefending) {
//            // ���⼭�� ���̽� ��� �÷����� �ʿ��մϴ�
//            std::set<std::shared_ptr<IceBlock>> iceBlocks;
//            CollectRemoveIceBlocks(matchedGroups, iceBlocks);
//        }
//
//        return true;
//    }
//    else {
//        // ��� �� �޺� ���� ����
//        if (stateInfo_.defenseCount >= 1 && !stateInfo_.isComboAttack) {
//            stateInfo_.isDefending = false;
//            stateInfo_.defenseCount = 0;
//        }
//
//        if (scoreInfo_.comboCount > 0) {
//            scoreInfo_.comboCount = 0;
//            NotifyEvent(GameCoreEvent::ComboChanged);
//        }
//
//        if (scoreInfo_.restScore > 0) {
//            scoreInfo_.restScore = 0;
//        }
//    }
//
//    // ���� ���� ���� Ȯ��
//    if (IsGameOver()) {
//        return true;
//    }
//
//    // ������ ��ȯ ó��
//    if (stateInfo_.shouldQuit) {
//        stateInfo_.currentPhase = GamePhase::Standing;
//    }
//    else {
//        stateInfo_.currentPhase = GamePhase::Playing;
//        stateInfo_.previousPhase = GamePhase::Playing;
//    }
//
//    return false;
//}
//
//void GameCore::ProcessMatchedBlocks(std::vector<std::vector<Block*>>& matchedGroups,
//    std::list<std::shared_ptr<Block>>& blockList)
//{
//    if (matchedGroups.empty()) {
//        return;
//    }
//
//    bool allProcessed = true;
//    for (const auto& group : matchedGroups) {
//        for (auto* block : group) {
//            if (block->GetState() != BlockState::PlayOut) {
//                allProcessed = false;
//                break;
//            }
//        }
//
//        if (!allProcessed) {
//            break;
//        }
//    }
//
//    if (allProcessed) 
//    {
//        for (const auto& group : matchedGroups) 
//        {
//            // �׷� �� ��� ��� ����
//            for (auto* block : group) 
//            {
//                int posX = block->GetPosIdx_X();
//                int posY = block->GetPosIdx_Y();
//
//                // ���� ���忡�� ����
//                boardBlocks_[posY][posX] = nullptr;
//
//                // ��� ����Ʈ���� ����
//                auto it = std::find_if(blockList.begin(), blockList.end(),
//                    [block](const std::shared_ptr<Block>& ptr) 
//                    {
//                        return ptr.get() == block;
//                    });
//
//                if (it != blockList.end()) 
//                {
//                    blockList.erase(it);
//                }
//            }
//        }
//
//        // ��� �׷� ó�� �� ��� ��ġ ������Ʈ
//        UpdateBlockPositions();
//
//        // �ӽ� ����� ��ġ �׷� �ʱ�ȭ
//        matchedGroups.clear();
//    }
//}
//
//
//bool GameCore::CanMoveLeft(int posX, int posY) const noexcept
//{
//    if (posX <= 0) {
//        return false;
//    }
//
//    // ���ʿ� �ٸ� ����� �ִ��� Ȯ��
//    return boardBlocks_[posY][posX - 1] == nullptr;
//}
//
//bool GameCore::CanMoveRight(int posX, int posY) const noexcept
//{
//    if (posX >= Constants::Board::BOARD_X_COUNT - 1) {
//        return false;
//    }
//
//    // �����ʿ� �ٸ� ����� �ִ��� Ȯ��
//    return boardBlocks_[posY][posX + 1] == nullptr;
//}
//
//bool GameCore::CanRotate(int posX, int posY, RotateState currentState) const noexcept
//{
//    // ȸ�� ���¿� ���� �˻� ��ġ ���
//    int checkX, checkY;
//
//    switch (currentState) {
//    case RotateState::Default:  // ���ο��� ���η� ȸ��
//        checkX = posX + 1;
//        checkY = posY;
//        break;
//
//    case RotateState::Right:    // ���ο��� ���η� ȸ��
//        checkX = posX;
//        checkY = posY - 1;
//        break;
//
//    default:
//        return false;
//    }
//
//    // üũ ��ġ�� ���� �ȿ� �ִ��� Ȯ��
//    if (checkX < 0 || checkX >= Constants::Board::BOARD_X_COUNT ||
//        checkY < 0 || checkY >= Constants::Board::BOARD_Y_COUNT) {
//        return false;
//    }
//
//    // üũ ��ġ�� �ٸ� ����� �ִ��� Ȯ��
//    return boardBlocks_[checkY][checkX] == nullptr;
//}
//
//Block* GameCore::GetBlockAt(int x, int y)
//{
//    if (x >= 0 && x < Constants::Board::BOARD_X_COUNT &&
//        y >= 0 && y < Constants::Board::BOARD_Y_COUNT) {
//        return boardBlocks_[y][x];
//    }
//    return nullptr;
//}
//
//void GameCore::SetBlockAt(int x, int y, Block* block)
//{
//    if (x >= 0 && x < Constants::Board::BOARD_X_COUNT &&
//        y >= 0 && y < Constants::Board::BOARD_Y_COUNT) {
//        boardBlocks_[y][x] = block;
//    }
//}
//
//void GameCore::ClearBoard()
//{
//    std::memset(boardBlocks_, 0, sizeof(boardBlocks_));
//}
//
//void GameCore::SetBoardBlocks(Block* blocks[Constants::Board::BOARD_Y_COUNT][Constants::Board::BOARD_X_COUNT])
//{
//    for (int y = 0; y < Constants::Board::BOARD_Y_COUNT; ++y) {
//        for (int x = 0; x < Constants::Board::BOARD_X_COUNT; ++x) {
//            boardBlocks_[y][x] = blocks[y][x];
//        }
//    }
//}
//
//void GameCore::CalculateIceBlockCount(const std::vector<std::vector<Block*>>& matchedGroups)
//{
//    // �޺� ���¿� ���� ī��Ʈ ������Ʈ
//    if (stateInfo_.previousPhase == GamePhase::Shattering) {
//        stateInfo_.defenseCount++;
//    }
//    else if (stateInfo_.previousPhase == GamePhase::Playing) {
//        stateInfo_.defenseCount = 1;
//    }
//
//    uint8_t linkBonus = 0;
//    uint8_t blockCount = 0;
//    uint8_t typeBonus = GetTypeBonus(matchedGroups.size());
//    short comboBonus = GetComboConstant(scoreInfo_.comboCount);
//
//    // ��ġ�� ��� �׷캰 ���ʽ� ���
//    for (const auto& group : matchedGroups) {
//        for (auto block : group) {
//            block->SetState(BlockState::Destroying);
//        }
//
//        linkBonus += GetLinkBonus(group.size());
//        blockCount += static_cast<uint8_t>(group.size());
//    }
//
//    // ���� ���� ���
//    int currentScore = ((blockCount * 10) * (comboBonus + linkBonus + typeBonus + 1));
//
//    // ���� ��� ī��Ʈ ���
//    scoreInfo_.addInterruptBlockCount = (currentScore + scoreInfo_.restScore) / GetMargin();
//    scoreInfo_.restScore = currentScore % GetMargin();
//    scoreInfo_.totalScore += currentScore;
//
//    // ���� ��� ���� ������Ʈ
//    if (scoreInfo_.totalInterruptBlockCount > 0) {
//        scoreInfo_.totalInterruptBlockCount -= scoreInfo_.addInterruptBlockCount;
//
//        if (scoreInfo_.totalInterruptBlockCount < 0) {
//            scoreInfo_.addInterruptBlockCount = std::abs(scoreInfo_.totalInterruptBlockCount);
//            stateInfo_.hasIceBlock = false;
//            scoreInfo_.totalInterruptBlockCount = 0;
//
//            if (scoreInfo_.addInterruptBlockCount > 0) {
//                scoreInfo_.totalEnemyInterruptBlockCount += scoreInfo_.addInterruptBlockCount;
//            }
//        }
//        else {
//            stateInfo_.hasIceBlock = true;
//        }
//    }
//    else {
//        if (scoreInfo_.addInterruptBlockCount > 0) {
//            scoreInfo_.totalEnemyInterruptBlockCount += scoreInfo_.addInterruptBlockCount;
//        }
//        stateInfo_.hasIceBlock = false;
//        scoreInfo_.totalInterruptBlockCount = 0;
//    }
//}