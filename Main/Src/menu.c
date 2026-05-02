/*
 * menu.c
 *
 *  Created on: 2026. 5. 1.
 *      Author: kth59
 */

#include "main.h"
#include "menu.h"
#include "st7789_lcd.h"
#include "bootloader.h"
#include "button.h"

/* 메뉴 항목 배열 초기화 */
MenuItem_t boot_menu_items[] = {
    { .name = "Boot Load", .pfnActionCallback = Boot_Loading },
    { .name = "Sec Load",  .pfnActionCallback = Boot_Loading },
    { .name = "Thd Load",  .pfnActionCallback = Boot_Loading },
    { .name = "F Load",    .pfnActionCallback = Boot_Loading },
};

/* 메인 메뉴 컨텍스트 초기화 */
MenuContext_t main_menu = {
    .category_name = "Main Menu",
    .pMenuItems    = boot_menu_items,
    .item_count    = 4,
    .prev_index    = 0,
    .cursor_index  = 0
};

/* 구조체 복사를 방지하기 위해 포인터로 넘겨받습니다. */
__STATIC_INLINE void Show_Menu(MenuContext_t *pCtx) {
    LCD_Printf(0, 0, "%s", pCtx->category_name);
    LCD_Printf(0, 1, "----------------");

    for (uint8_t i = 0; i < pCtx->item_count; i++) {
        if (i == pCtx->cursor_index) {
            LCD_Set_Color(BLACK, WHITE); // 커서 위치 반전 효과
        } else {
            LCD_Set_Color(WHITE, BLACK); // 일반 텍스트
        }
        // 포인터 배열 접근: pCtx->pMenuItems[i].name
        LCD_Printf(0, 2 + i, "%d. %s ", i + 1, pCtx->pMenuItems[i].name);
    }
    LCD_Set_Color(WHITE, BLACK); // 기본 색상으로 원복
}

__STATIC_INLINE void Select_Menu(MenuContext_t *pCtx) {
    UserInput_t bt = Button_GetInput();

    switch(bt) {
        case INPUT_CMD_K_SINGLE:
            pCtx->cursor_index = (pCtx->cursor_index == 0) ? (pCtx->item_count - 1) : (pCtx->cursor_index - 1);
            break;

        case INPUT_CMD_K_DOUBLE:
            pCtx->cursor_index = (pCtx->cursor_index == (pCtx->item_count - 1)) ? 0 : (pCtx->cursor_index + 1);
            break;

        case INPUT_CMD_K_HOLD:
            // 1. 현재 커서 위치를 가져옴
            uint8_t selected = pCtx->cursor_index;
            // 2. NULL 포인터 참조 에러(HardFault) 방지 처리
            if (pCtx->pMenuItems[selected].pfnActionCallback != NULL) {
                pCtx->pMenuItems[selected].pfnActionCallback(); // 올바른 함수 호출
            }
            break;

        default:
            break;
    }
}

/* 메인 루틴에서 지속적으로 호출될 함수 (이름도 ProcessLoop으로 더 명확하게 변경) */
void Menu_ProcessLoop(MenuContext_t *pCtx) {
    // 1. 버튼 입력 확인 및 커서 조작
    Select_Menu(pCtx);

    // 2. 변경사항 화면 렌더링
    // (최적화를 원한다면 pCtx->prev_index != pCtx->cursor_index 일 때만 그리도록 조건문을 넣으셔도 좋습니다)
    Show_Menu(pCtx);

    // 3. 과거 상태 업데이트
    pCtx->prev_index = pCtx->cursor_index;
}
