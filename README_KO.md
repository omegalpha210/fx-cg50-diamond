# CASIO fx-CG50용 DIAMOND

<img src="assets/icon-uns.png" width="92" height="64" alt="DIAMOND 앱 아이콘">

**내 말 10개를 반대편 진영으로! 73칸 별 모양 보드에서 AI와 겨루세요.**

CASIO fx-CG50에서 즐기는 한국식 다이아몬드 게임입니다.
내 말은 **RED (YOU)**입니다. 2P에서는 GREEN과, 3P에서는 서로 독립적으로
판단하는 YELLOW·GREEN AI와 대결합니다. EASY·NORMAL·HARD 난이도와
이동 힌트, 확대, 무르기, 이어하기를 지원합니다.

[English](README.md) · [beta.4 다운로드](https://github.com/omegalpha210/fx-cg50-diamond/releases/tag/v0.1.0-beta.4) · [전체 규칙](docs/GAME_RULES.md)

| 2P · 나와 GREEN | 3P · 나와 YELLOW·GREEN |
|---|---|
| ![V2 규칙의 2인 시작 보드](docs/screenshots/2p-overview.png) | ![V2 규칙의 3인 시작 보드](docs/screenshots/3p-overview.png) |

**아래는 새 진영 규칙이 적용된 beta.4 V2 화면입니다.** 실제 게임과 같은
렌더러로 PC에서 캡처한 396 × 224 이미지이며, 계산기를 촬영한 사진은 아닙니다.
진영 규칙과 승리 예시는 설명을 위해 구성하고 엔진으로 검증한 V2 배치입니다.
[캡처별 설명](docs/screenshots/README.md)에서 구분을 확인할 수 있습니다.

## 이렇게 플레이하세요

1. **EXE**로 내 RED 말 하나를 선택합니다.
2. 인접한 빈칸으로 한 칸 이동하거나, 바로 옆 말을 넘어 그 뒤의 빈칸으로
   점프합니다. 연속 점프 중 방향을 바꾸거나 원하는 착지점에서 끝낼 수 있습니다.
3. 반대편 목표 진영 10칸을 먼저 채우면 승리합니다. 내 목표는 화면 위쪽 진영입니다.

모든 말의 능력은 같습니다. 잡기나 왕말은 없습니다. 한 턴에 한 칸 이동과
점프를 섞을 수 없습니다. 점프 도중 되돌아가기는 허용하지만, 출발점으로
돌아와 턴을 끝낼 수는 없습니다. 승리 전에는 목표 진영에서 다시 나올 수 있습니다.

## 새 진영 규칙, 화면으로 보기

**HOME**은 출발 진영, **GOAL**은 반대편 목표 진영입니다. 중앙을 향한
4칸 경계열도 각 진영에 포함되며, 일부 모서리 칸은 두 진영이 공유합니다.
연속 점프의 중간 착지까지 다음 순서로 판정합니다.

| 도착 칸의 소속 | 들어갈 수 있나요? |
|---|---|
| 내 HOME 또는 GOAL에 속함 — 공유 모서리도 포함 | **가능.** 빈칸 여부와 일반 이동 조건도 충족해야 합니다. |
| 내 진영에는 속하지 않고, 현재 참가 중인 상대의 HOME 또는 GOAL에 속함 | **불가.** |
| 중립 칸 또는 활성 상대 진영과 겹치지 않는 비참가 색의 진영 | **가능.** 빈칸 여부와 일반 이동 조건도 충족해야 합니다. |

| 상대 진영에만 속한 칸: 진입 금지 | 내 목표이기도 한 공유 모서리: 진입 가능 |
|---|---|
| ![상대 진영 진입 시 OPPONENT CAMP 경고가 표시되는 V2 화면](docs/screenshots/opponent-camp-overview.png) | ![내 위쪽 목표 진영의 공유 모서리가 청록색 합법 도착점으로 표시된 V2 화면](docs/screenshots/shared-red-goal-allowed.png) |
| 커서가 가리키는 칸은 GREEN의 목표 경계입니다. **OPPONENT CAMP**가 뜨고 그 칸은 청록색 도착점으로 표시되지 않습니다. | 위쪽 진영 왼쪽 모서리는 RED GOAL이자 YELLOW HOME입니다. RED에게는 자기 목표이므로 들어갈 수 있습니다. |

공유 칸에 특정 색의 독점 소유권을 부여하지 않고 **움직이는 플레이어의 소속**을
기준으로 판단합니다. 상대 진영 출입 금지와 4칸 경계열 포함은 사용자 제공 실물
설명서에 근거하며, 공유 칸에서 자기 진영을 먼저 인정하는 것은
[프로젝트 해석](docs/RULE_SOURCES.md)입니다. 실제 73칸 보드 구조는 그대로입니다.
2P에서는 비참가 색인 YELLOW 진영이라는 이유만으로 진입을 막지 않습니다.

![자기 진영 우선과 상대 진영 진입 금지를 설명하는 게임 내 V2 규칙 화면](docs/screenshots/rules-v2-camps.png)

## 이동 힌트와 확대

**ASSIST ON**이면 합법적인 도착 칸을 청록색으로 보여줍니다. 말을 선택하고
원하는 도착점으로 커서를 옮긴 뒤 EXE로 이동합니다. **F5**를 누르면 선택을
유지한 채 전체 보기와 확대 보기를 전환합니다.

| 전체 보기 · RED 말 선택 | F5 · 같은 선택을 확대 |
|---|---|
| ![V2 전체 보기에서 합법 도착점이 표시된 RED 선택 화면](docs/screenshots/selected-overview.png) | ![같은 V2 선택과 합법 도착점을 확대한 화면](docs/screenshots/selected-zoom.png) |

ASSIST OFF로 힌트를 숨길 수 있습니다. 점프 경로는 최단 대표 경로 하나를
보여줍니다. 사용자 이동·힌트·AI는 모두 같은 엔진의 합법성 판정을 사용합니다.
AI의 최종 수는 이동 애니메이션으로 보여주며, 방향 흔적은 다음 합법적인 내 이동까지 남습니다.

## 마지막 한 칸까지 채우면 승리

오른쪽 숫자는 각 플레이어가 목표 진영에 넣은 말의 수입니다.
아래 V2 예시 배치에서 GREEN은 **9/10**입니다. 실제 AI가 마지막 말을 넣으면
이동 재생이 끝난 뒤 **10/10**과 승리 안내가 표시됩니다.

| 목표까지 한 개 남음 | 마지막 말 도착 · GREEN 승리 |
|---|---|
| ![GREEN이 목표 진영 9칸을 채운 V2 종반 예시](docs/screenshots/endgame-nine-green.png) | ![GREEN AI가 목표 10칸을 채워 승리한 V2 화면](docs/screenshots/endgame-green-wins.png) |

EASY로 가볍게 시작하고 NORMAL·HARD로 난이도를 높일 수 있습니다.
3P의 두 AI는 각자의 승리를 목표로 합니다. Beta.4는 종반의 남은 목표 칸 접근을
개선했으며, 모든 난이도에서 즉시 가능한 합법적 승리를 먼저 선택합니다.
[AI 설계와 측정 한계](docs/AI_DESIGN.md).

## 설치·시작·이어하기

[beta.4 릴리스](https://github.com/omegalpha210/fx-cg50-diamond/releases/tag/v0.1.0-beta.4)에서
**DIAMOND.g3a**와 **SHA256SUMS.txt**를 받습니다. 아래 명령으로 체크섬을 확인하고,
USB로 계산기 저장 메모리 최상위에 G3A를 복사한 뒤 안전하게 연결을 해제합니다.
CASIO Main Menu에서 DIAMOND를 실행합니다.

```sh
shasum -a 256 -c SHA256SUMS.txt
```

| 1 · 2P 또는 3P 선택 | 2 · 설정을 고르고 시작 |
|---|---|
| ![2인과 3인을 선택하는 PLAYER 화면](docs/screenshots/player.png) | ![이어하기, 새 게임, 난이도, 순서, 힌트가 있는 GAME SETUP 화면](docs/screenshots/setup-3p-resume.png) |

- **PLAYER:** 2P/3P를 고른 뒤 EXE/F6 NEXT로 진행합니다. **F1 RESUME**은
  다른 인원 수의 타일을 선택한 상태에서도 저장된 게임 하나를 이어갑니다.
- **GAME SETUP:** 위/아래로 행을 고르고 왼쪽/오른쪽으로 난이도, 선공/내 차례,
  Assist를 바꿉니다. 선택은 양 끝에서 멈춥니다.
- **NEW GAME:** RESUME 이외의 행에서 EXE/F6 OPEN을 누르면 현재 설정으로
  새 게임을 시작합니다. 같은 인원 수의 저장이 있으면 RESUME 행이 추가됩니다.

**새 규칙으로 플레이하려면 NEW GAME을 선택하세요.** 기존 V1 저장을
RESUME하거나 RESTART하면 원래 규칙을 유지하며, 규칙 화면에 **LEGACY RULES**라고
표시됩니다. 새 V2 게임에는 위 사진의 진영 규칙이 적용됩니다.

## 게임 중 조작

| 키 | 동작 |
|---|---|
| 방향키 | 전체/확대 보기의 커서 이동 |
| EXE / F6 | 내 RED 말을 선택하고 합법적인 도착 칸으로 이동 |
| EXIT | 선택 해제 → 확대 해제 → 저장 후 GAME SETUP |
| F1 | 확인 후 재시작 |
| F2 | 내 결정 한 번과 이후 AI 응답을 한 번 되돌리기 |
| F4 | 스크롤 가능한 규칙 |
| F5 | 전체 보기 / 확대 전환 |
| MENU | 저장 후 CASIO Main Menu |
| SHIFT + AC/ON | 저장 후 전원 종료 |

진행 중인 게임 하나를 체크섬이 있는 A/B 파일에 번갈아 저장합니다.
커서 이동이나 AI 탐색 중에는 플래시에 쓰지 않습니다. [저장과 복구](docs/STORAGE_FORMAT.md).

**실험적 베타 — HARDWARE TEST REQUIRED.** 호스트 테스트와 패키지 검사는
통과했습니다. 실제 계산기의 화면·저장·전원·AI 응답 시간·USB 연결 동작은
[실기 확인](docs/HARDWARE_RETEST.md)이 필요합니다. USB 처리는 실험적이며,
beta.4가 USB 문제 해결을 보장하지는 않습니다.
[Beta.4 검증](docs/BETA4_AUDIT.md) · [USB 감사](docs/USB_LIFECYCLE_AUDIT.md).

## 빌드와 검증

호스트는 CMake·C11 컴파일러·Python 3, 네이티브 빌드는 fxSDK·gint 2.11·SH
크로스 컴파일러가 필요합니다. `DIAMOND_SDK_ROOT`로 설치 위치를 지정하며
기본값은 `$HOME/.local/diffeq-sdk`입니다.

```sh
bash tools/test.sh
bash tools/build.sh
cmake -S . -B build/ubsan -DDG_HOST=ON -DDG_SANITIZE=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build/ubsan -j8
ctest --test-dir build/ubsan --output-on-failure
```

산출물은 `dist/DIAMOND.g3a`와 `dist/SHA256SUMS.txt`입니다. Pillow는 폰트·아이콘
재생성 및 캡처 변환에만 필요합니다. [개발 안내](docs/DEVELOPMENT.md),
[검증](docs/BETA_VALIDATION.md), [메모리](docs/MEMORY.md),
[실제 렌더러 캡처](docs/screenshots/README.md),
[공개 이력](docs/PUBLICATION.md)에 근거와 한계를 기록했습니다.
[UI polish 감사](docs/UI_POLISH_BETA2.md)는 번호 행·점 애니메이션·RESUME/EXIT 전환과
보드 겹침 검사를 기록합니다.
호스트 ms를 계산기 응답 시간으로 해석하지 않습니다.

## 출처와 라이선스

규칙은 사용자가 전사한 실물 한국어 설명서와 [코리아보드게임즈](https://www.koreaboardgames.com/magazine/menuDetail?boardCd=contents&postNo=314)와
[한국어 위키백과](https://ko.wikipedia.org/wiki/다이아몬드_게임)를 참고해 직접 요약했습니다.
[출처별 결정](docs/RULE_SOURCES.md)과 [격자 증명](docs/BOARD_GEOMETRY.md)을 확인할 수
있습니다. 실제 73칸 별 격자의 10칸 진영은 경계 모서리 6칸을 공유하며, 모든
진영 밖에는 19칸이 있습니다.

자체 게임 코드·생성 보드·아이콘·자체 캡처는 [MIT](LICENSE)입니다.
차용한 도우미, gint 폰트, 런타임은 [별도 고지](THIRD_PARTY_NOTICES.md)를
유지합니다. 기사·보드 사진·Sokoban 맵·비공개 참조 화면은 배포하지 않습니다.
CASIO와의 제휴 또는 공식 보증을 뜻하지 않습니다.

현재 AI 애니메이션·흔적의 상태 수명과 중단 동작은
[AI_MOVE_VISUALIZATION](docs/AI_MOVE_VISUALIZATION.md), 실제 색상 값은
[UI_COLOR_AUDIT](docs/UI_COLOR_AUDIT.md)에 기록했습니다.
