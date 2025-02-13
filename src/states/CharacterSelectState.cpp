// states/CharacterSelectState.cpp 

CharacterSelectState::CharacterSelectState() {
    characters.resize(BOARD_HEIGHT * BOARD_WIDTH);
}

bool CharacterSelectState::Init() {
    if (isInitialized()) {
        return false;
    }

    try {
        if (!loadBackgrounds() ||
            !loadCharacterResources() ||
            !loadUIElements()) {
            return false;
        }

        initializeCharacters();
        initialized = true;
        return true;
    }
    catch (const std::exception& e) {
        Logger::error("CharacterSelectState initialization failed: {}", e.what());
        return false;
    }
}

void CharacterSelectState::Enter() {
    // 네트워크 역할에 따른 초기 위치 설정
    if (NetworkManager::getInstance().isServer()) {
        currentPos = { 0, 0 };
        enemyPos = { 6, 0 };
    }
    else {
        currentPos = { 6, 0 };
        enemyPos = { 0, 0 };
    }

    isSelected = false;
    isEnemySelected = false;

    if (startButton) {
        startButton->setVisible(false);
    }
}

void CharacterSelectState::HandleEvent(const SDL_Event& event) {
    if (startButton) {
        startButton->handleEvent(event);
    }

    switch (event.type) {
    case SDL_KEYDOWN:
        if (!isSelected) {
            updateSelection(event);
        }
        break;
    case SDL_SYSWMEVENT:
        handleNetworkEvent(event.syswm);
        break;
    }
}

void CharacterSelectState::updateSelection(const SDL_Event& event) {
    Position newPos = currentPos;

    switch (event.key.keysym.sym) {
    case SDLK_UP:
        if (newPos.y > 0) newPos.y--;
        break;
    case SDLK_DOWN:
        if (canMoveDown(newPos)) newPos.y++;
        break;
    case SDLK_LEFT:
        if (canMoveLeft(newPos)) newPos.x--;
        break;
    case SDLK_RIGHT:
        if (canMoveRight(newPos)) newPos.x++;
        break;
    case SDLK_SPACE:
        handleCharacterSelection();
        return;
    }

    if (newPos != currentPos) {
        currentPos = newPos;
        NetworkManager::getInstance().sendCharacterMove(currentPos.x, currentPos.y);
        updateUIState();
    }
}