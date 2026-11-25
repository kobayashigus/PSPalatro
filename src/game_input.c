#include "global.h"

void game_input_play_button_sfx()
{
    if (input_was_button_pressed(INPUT_BUTTON_CIRCLE))
    {
        audio_play_sfx(AUDIO_SFX_CANCEL);
    }
    else if (input_was_button_pressed(INPUT_BUTTON_UP) ||
             input_was_button_pressed(INPUT_BUTTON_DOWN) ||
             input_was_button_pressed(INPUT_BUTTON_LEFT) ||
             input_was_button_pressed(INPUT_BUTTON_RIGHT))
    {
        audio_play_sfx(AUDIO_SFX_HIGHLIGHT);
    }
    else if (input_was_button_pressed(INPUT_BUTTON_CROSS) ||
             input_was_button_pressed(INPUT_BUTTON_SQUARE) ||
             input_was_button_pressed(INPUT_BUTTON_TRIANGLE) ||
             input_was_button_pressed(INPUT_BUTTON_LEFT_TRIGGER) ||
             input_was_button_pressed(INPUT_BUTTON_RIGHT_TRIGGER) ||
             input_was_button_pressed(INPUT_BUTTON_START) ||
             input_was_button_pressed(INPUT_BUTTON_SELECT))
    {
        audio_play_sfx(AUDIO_SFX_BUTTON);
    }
}

// Passo direcional com repetição estilo cardtest: age na hora do aperto,
// espera 0.35s parado e repete a cada 0.09s enquanto segurar a setinha.
// Retorna -1/0/+1 no frame em que o passo deve acontecer.
static float s_lr_rep_t = 0.0f;
static int s_lr_rep_dir = 0;

static int game_input_repeat_lr()
{
    int left_p = input_was_button_pressed(INPUT_BUTTON_LEFT) ? 1 : 0;
    int right_p = input_was_button_pressed(INPUT_BUTTON_RIGHT) ? 1 : 0;
    int left_d = input_is_button_down(INPUT_BUTTON_LEFT) ? 1 : 0;
    int right_d = input_is_button_down(INPUT_BUTTON_RIGHT) ? 1 : 0;
    const float dt = 1.0f / 60.0f;
    if (left_p && !right_p)
    {
        s_lr_rep_dir = -1;
        s_lr_rep_t = 0.35f;
        return -1;
    }
    if (right_p && !left_p)
    {
        s_lr_rep_dir = 1;
        s_lr_rep_t = 0.35f;
        return 1;
    }
    if (left_d && !right_d && s_lr_rep_dir == -1)
    {
        s_lr_rep_t -= dt;
        if (s_lr_rep_t <= 0.0f)
        {
            s_lr_rep_t = 0.09f;
            return -1;
        }
        return 0;
    }
    if (right_d && !left_d && s_lr_rep_dir == 1)
    {
        s_lr_rep_t -= dt;
        if (s_lr_rep_t <= 0.0f)
        {
            s_lr_rep_t = 0.09f;
            return 1;
        }
        return 0;
    }
    s_lr_rep_dir = 0;
    return 0;
}

void game_input_update_joker(bool no_input)
{
    if (input_was_button_released(INPUT_BUTTON_CROSS) && g_game_state.moving_card)
    {
        g_game_state.moving_card = false;
    }

    if (!no_input)
    {
        // Passo com repetição estilo cardtest (0.35s parado, 0.09s segurando).
        int lr = game_input_repeat_lr();
        if (lr > 0)
        {
            if (g_game_state.highlighted_item < g_game_state.jokers.joker_count - 1)
            {
                if (input_is_button_down(INPUT_BUTTON_CROSS))
                {
                    g_game_state.moving_card = true;
                    g_game_state.cross_pressed = false;
                    automated_event_set(AUTOMATED_EVENT_HAND_CARD_SWAP, 3, g_game_state.highlighted_item, g_game_state.highlighted_item + 1, EVENT_CARD_LOCATION_JOKER);
                }
                else
                {
                    g_game_state.highlighted_item++;
                }
            }
            else
            {
                if (g_game_state.consumables.item_count > 0)
                {
                    g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_CONSUMABLES;
                    g_game_state.highlighted_item = 0;
                }
            }
        }
        else if (lr < 0)
        {
            if (g_game_state.highlighted_item > 0)
            {
                if (input_is_button_down(INPUT_BUTTON_CROSS))
                {
                    g_game_state.moving_card = true;
                    g_game_state.cross_pressed = false;
                    automated_event_set(AUTOMATED_EVENT_HAND_CARD_SWAP, 3, g_game_state.highlighted_item, g_game_state.highlighted_item - 1, EVENT_CARD_LOCATION_JOKER);
                }
                else
                {
                    g_game_state.highlighted_item--;
                }                
            }
        }
        
        if (input_was_button_pressed(INPUT_BUTTON_CROSS))
        {
            g_game_state.cross_pressed = true;
        }

        if (input_was_button_pressed(INPUT_BUTTON_LEFT_TRIGGER))
        {
            game_sell_joker();
        }

        if (input_was_button_pressed(INPUT_BUTTON_DOWN))
        {
            if (g_game_state.stage == GAME_STAGE_INGAME && g_game_state.sub_stage == GAME_SUBSTAGE_INGAME_PICK_HAND)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_HAND;
                g_game_state.highlighted_item = 0;
            }
            else if (g_game_state.stage == GAME_STAGE_BLINDS)
            {
                g_game_state.highlighted_item = 0;
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_BLIND;
            }
            else if (g_game_state.stage == GAME_STAGE_SHOP && g_game_state.sub_stage == GAME_SUBSTAGE_SHOP_MAIN)
            {
                g_game_state.highlighted_item = 0;
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_NEXT_ROUND;
            }
            else if (g_game_state.stage == GAME_STAGE_SHOP && g_game_state.sub_stage == GAME_SUBSTAGE_SHOP_BOOSTER_HAND)
            {
                g_game_state.highlighted_item = 0;
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_HAND;
            }
            else if (g_game_state.stage == GAME_STAGE_SHOP && g_game_state.sub_stage == GAME_SUBSTAGE_SHOP_BOOSTER_NO_HAND)
            {
                g_game_state.highlighted_item = game_util_get_first_shop_booster_item_index();
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_BOOSTER_ITEMS;
            }
        }
    }
}

void game_input_update_consumables(bool no_input)
{
    if (input_was_button_released(INPUT_BUTTON_CROSS) && g_game_state.moving_card)
    {
        g_game_state.moving_card = false;
    }

    if (!no_input)
    {
        // Passo com repetição estilo cardtest (0.35s parado, 0.09s segurando).
        int lr = game_input_repeat_lr();
        if (lr > 0)
        {
            if (g_game_state.highlighted_item < g_game_state.consumables.item_count - 1)
            {
                if (input_is_button_down(INPUT_BUTTON_CROSS))
                {
                    g_game_state.moving_card = true;
                    g_game_state.cross_pressed = false;
                    automated_event_set(AUTOMATED_EVENT_HAND_CARD_SWAP, 3, g_game_state.highlighted_item, g_game_state.highlighted_item + 1, EVENT_CARD_LOCATION_CONSUMABLES);
                }
                else
                {
                    g_game_state.highlighted_item++;
                }                
            }
        }
        else if (lr < 0)
        {
            if (g_game_state.highlighted_item > 0)
            {
                if (input_is_button_down(INPUT_BUTTON_CROSS))
                {
                    g_game_state.moving_card = true;
                    g_game_state.cross_pressed = false;
                    automated_event_set(AUTOMATED_EVENT_HAND_CARD_SWAP, 3, g_game_state.highlighted_item, g_game_state.highlighted_item - 1, EVENT_CARD_LOCATION_CONSUMABLES);
                }
                else
                {
                    g_game_state.highlighted_item--;
                }                
            }
            else
            {
                if (g_game_state.jokers.joker_count > 0)
                {
                    g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_JOKERS;
                    g_game_state.highlighted_item = g_game_state.jokers.joker_count - 1;
                }
            }
        }
        
        if (input_was_button_pressed(INPUT_BUTTON_CROSS))
        {                 
            g_game_state.cross_pressed = true;
        }

        if (input_was_button_pressed(INPUT_BUTTON_LEFT_TRIGGER))
        {
            game_sell_consumable();
        }
        else if (input_was_button_pressed(INPUT_BUTTON_RIGHT_TRIGGER))
        {
            game_use_consumable();
        }

        if (input_was_button_pressed(INPUT_BUTTON_DOWN))
        {
            if (g_game_state.stage == GAME_STAGE_INGAME && g_game_state.sub_stage == GAME_SUBSTAGE_INGAME_PICK_HAND)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_HAND;
                g_game_state.highlighted_item = 0;
            }
            else if (g_game_state.stage == GAME_STAGE_BLINDS)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_BLIND;
            }
            else if (g_game_state.stage == GAME_STAGE_SHOP && g_game_state.sub_stage == GAME_SUBSTAGE_SHOP_MAIN)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_NEXT_ROUND;
            }
        }
    }
}

void game_input_update_hand(bool no_input)
{
    if (input_was_button_released(INPUT_BUTTON_CROSS) && g_game_state.moving_card)
    {
        g_game_state.moving_card = false;
    }

    if (!no_input)
    {
        g_game_state.show_highlighted_card = true;

        // Passo com repetição estilo cardtest (0.35s parado, 0.09s segurando).
        int lr = game_input_repeat_lr();
        if (lr > 0)
        {
            if (g_game_state.highlighted_item < g_game_state.hand.card_count - 1)
            {
                if (input_is_button_down(INPUT_BUTTON_CROSS))
                {
                    g_game_state.moving_card = true;
                    g_game_state.cross_pressed = false;
                    automated_event_set(AUTOMATED_EVENT_HAND_CARD_SWAP, 3, g_game_state.highlighted_item, g_game_state.highlighted_item + 1, EVENT_CARD_LOCATION_HAND);
                }
                else
                {
                    g_game_state.highlighted_item++;
                }
            }
            else
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_DECK;
            }
        }
        else if (lr < 0)
        {
            if (g_game_state.highlighted_item > 0)
            {
                if (input_is_button_down(INPUT_BUTTON_CROSS))
                {
                    g_game_state.moving_card = true;
                    g_game_state.cross_pressed = false;
                    automated_event_set(AUTOMATED_EVENT_HAND_CARD_SWAP, 3, g_game_state.highlighted_item, g_game_state.highlighted_item - 1, EVENT_CARD_LOCATION_HAND);
                }
                else
                {
                    g_game_state.highlighted_item--;
                }
            }
            // (Run Info agora e acessado pelo SELECT)
        }

        if (input_was_button_pressed(INPUT_BUTTON_CROSS))
        {
            g_game_state.cross_pressed = true;
        }

        if (input_was_button_released(INPUT_BUTTON_CROSS) && g_game_state.cross_pressed)
        {
            g_game_state.cross_pressed = false;
            if (!g_game_state.moving_card)
            {
                if (game_util_is_boss_blind_active(BOSS_BLIND_CERULEAN_BELL) &&
                    g_game_state.hand.cards[g_game_state.highlighted_item] == g_game_state.boss_forced_selected_card &&
                    g_game_state.hand.cards[g_game_state.highlighted_item]->selected)
                {
                    game_set_card_hand_positions();
                    return;
                }
                if (g_game_state.hand.cards[g_game_state.highlighted_item]->selected || 
                    (!g_game_state.hand.cards[g_game_state.highlighted_item]->selected && g_game_state.selected_cards_count < 5))
                {
                    g_game_state.hand.cards[g_game_state.highlighted_item]->selected = !g_game_state.hand.cards[g_game_state.highlighted_item]->selected;
                    if (g_game_state.hand.cards[g_game_state.highlighted_item]->selected)
                    {
                        g_game_state.selected_cards_count++;
                        g_game_state.hand.cards[g_game_state.highlighted_item]->draw.tilt_wv += 4.3f;
                    }
                    else
                    {
                        g_game_state.selected_cards_count--;
                    }

                    struct Card *cards[5];

                    int current_card = 0;
                    for (int i = 0; i < g_game_state.hand.card_count; i++)
                    {
                        if (g_game_state.hand.cards[i]->selected)
                        {
                            cards[current_card++] = g_game_state.hand.cards[i];
                        }
                    }

                    int scoring_cards[5];
                    int scoring_cards_count = 0;

                    g_game_state.current_poker_hand = game_get_selected_poker_hand(cards, g_game_state.selected_cards_count, scoring_cards, &scoring_cards_count);
                    if (g_game_state.current_poker_hand == GAME_POKER_HAND_NONE)
                    {
                        g_game_state.current_base_chips = 0;
                        g_game_state.current_base_mult = 0;
                    }
                    else
                    {
                        g_game_state.current_base_chips = g_game_state.poker_hand_base_chips[g_game_state.current_poker_hand];
                        g_game_state.current_base_mult = g_game_state.poker_hand_base_mult[g_game_state.current_poker_hand];
                    }
                }

                game_set_card_hand_positions();
            }
        }
        else if (input_was_button_released(INPUT_BUTTON_TRIANGLE))
        {
            if (g_game_state.stage == GAME_STAGE_INGAME)
            {
                game_discard_selected_cards();
            }
            else if (g_game_state.stage == GAME_STAGE_SHOP && g_game_state.sub_stage == GAME_SUBSTAGE_SHOP_BOOSTER_HAND)
            {
                g_game_state.highlighted_item = 0;
                game_go_to_stage(GAME_STAGE_SHOP, GAME_SUBSTAGE_SHOP_MAIN);
            }
        }
        else if (input_was_button_released(INPUT_BUTTON_SQUARE))
        {
            if (g_game_state.stage == GAME_STAGE_INGAME)
            {
                if (game_util_can_play_selected_hand())
                {
                    g_game_state.highlighted_item = 0;
                    automated_event_set(AUTOMATED_EVENT_SCORE, 0);
                    g_game_state.current_hands--;
                }
            }
        }
        else if (input_was_button_released(INPUT_BUTTON_CIRCLE))
        {            
            for (int i = 0; i < g_game_state.hand.card_count; i++)
            {
                g_game_state.hand.cards[i]->selected =
                    game_util_is_boss_blind_active(BOSS_BLIND_CERULEAN_BELL) &&
                    g_game_state.hand.cards[i] == g_game_state.boss_forced_selected_card;
            }
            g_game_state.selected_cards_count =
                game_util_is_boss_blind_active(BOSS_BLIND_CERULEAN_BELL) &&
                g_game_state.boss_forced_selected_card != NULL ? 1 : 0;
            if (g_game_state.selected_cards_count == 0)
            {
                g_game_state.current_poker_hand = GAME_POKER_HAND_NONE;
                g_game_state.current_base_chips = 0;
                g_game_state.current_base_mult = 0;
            }
            else
            {
                struct Card *cards[5];
                cards[0] = g_game_state.boss_forced_selected_card;
                int scoring_cards[5];
                int scoring_cards_count = 0;
                g_game_state.current_poker_hand = game_get_selected_poker_hand(cards, 1, scoring_cards, &scoring_cards_count);
                g_game_state.current_base_chips = g_game_state.poker_hand_base_chips[g_game_state.current_poker_hand];
                g_game_state.current_base_mult = g_game_state.poker_hand_base_mult[g_game_state.current_poker_hand];
            }
            g_game_state.highlighted_item = 0;
            game_set_card_hand_positions();
        }

        if (input_was_button_pressed(INPUT_BUTTON_UP))
        {
            if (g_game_state.jokers.joker_count > 0)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_JOKERS;
                g_game_state.highlighted_item = 0;
            }
            else if (g_game_state.consumables.item_count > 0)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_CONSUMABLES;
                g_game_state.highlighted_item = 0;
            }
        }
        else if (input_was_button_pressed(INPUT_BUTTON_DOWN))
        {
            if (g_game_state.stage == GAME_STAGE_INGAME)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SORT_HAND_RANK;
                g_game_state.highlighted_item = 0;
            }
            else if(g_game_state.stage == GAME_STAGE_SHOP && g_game_state.sub_stage == GAME_SUBSTAGE_SHOP_BOOSTER_HAND)
            {
                g_game_state.highlighted_item = game_util_get_first_shop_booster_item_index();
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_BOOSTER_ITEMS;
            }
        }
    }
    else
    {
        g_game_state.show_highlighted_card = false;
    }
}

void game_input_update_blind(bool no_input)
{
    // Self-heal: foco invalido nesta tela mata todo o input (o despacho
    // erra o handler) mas START/SELECT sobrevivem (globais) — sintoma
    // exato de "tela viva, resto morto". Foco valido aqui: BLIND, JOKERS,
    // CONSUMABLES, DECK (navegacao) + overlays (tratados antes do despacho).
    switch (g_game_state.input_focused_zone)
    {
        case INPUT_FOCUSED_ZONE_BLIND:
        case INPUT_FOCUSED_ZONE_JOKERS:
        case INPUT_FOCUSED_ZONE_CONSUMABLES:
        case INPUT_FOCUSED_ZONE_DECK:
        case INPUT_FOCUSED_ZONE_RUN_INFO_CLOSE:
        case INPUT_FOCUSED_ZONE_DECK_INFO_CLOSE:
        case INPUT_FOCUSED_ZONE_DECK_INFO_CARD:
            break;
        default:
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_BLIND;
            g_game_state.blind_focused_action = BLIND_ACTION_SELECT;
            break;
    }

    if (!no_input)
    {
        if (g_game_state.blind_tag_description_open)
        {
            if (input_was_button_pressed(INPUT_BUTTON_CROSS) ||
                input_was_button_pressed(INPUT_BUTTON_CIRCLE))
            {
                g_game_state.blind_tag_description_open = false;
                audio_play_sfx(AUDIO_SFX_CANCEL);
            }
            return;
        }

        if (input_was_button_pressed(INPUT_BUTTON_CROSS))
        {
            if (g_game_state.blind_focused_action == BLIND_ACTION_SKIP &&
                g_game_state.blind != GAME_BLIND_BOSS)
            {
                game_skip_current_blind();
                audio_play_sfx(AUDIO_SFX_BUTTON);
            }
            else if (g_game_state.blind_focused_action == BLIND_ACTION_TAG &&
                     g_game_state.blind != GAME_BLIND_BOSS)
            {
                g_game_state.blind_tag_description_open = true;
                audio_play_sfx(AUDIO_SFX_BUTTON);
            }
            else
            {
                automated_event_push(AUTOMATED_EVENT_SELECT_BLIND, 0);
            }
        }

        if (input_was_button_pressed(INPUT_BUTTON_UP))
        {
            if (g_game_state.blind_focused_action != BLIND_ACTION_SELECT)
            {
                g_game_state.blind_focused_action = BLIND_ACTION_SELECT;
                audio_play_sfx(AUDIO_SFX_HIGHLIGHT);
            }
            else if (g_game_state.jokers.joker_count > 0)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_JOKERS;
                g_game_state.highlighted_item = 0;
            }
            else if (g_game_state.consumables.item_count > 0)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_CONSUMABLES;
                g_game_state.highlighted_item = 0;
            }
        }

        if (input_was_button_pressed(INPUT_BUTTON_DOWN) &&
            g_game_state.blind != GAME_BLIND_BOSS)
        {
            g_game_state.blind_focused_action = BLIND_ACTION_TAG;
            audio_play_sfx(AUDIO_SFX_HIGHLIGHT);
        }

        if (input_was_button_pressed(INPUT_BUTTON_RIGHT))
        {
            if (g_game_state.blind_focused_action == BLIND_ACTION_TAG)
            {
                g_game_state.blind_focused_action = BLIND_ACTION_SKIP;
                audio_play_sfx(AUDIO_SFX_HIGHLIGHT);
            }
            else if (g_game_state.blind_focused_action == BLIND_ACTION_SKIP)
            {
                g_game_state.blind_focused_action = BLIND_ACTION_TAG;
                audio_play_sfx(AUDIO_SFX_HIGHLIGHT);
            }
            else
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_DECK;
            }
        }
        else if (input_was_button_pressed(INPUT_BUTTON_LEFT))
        {
            if (g_game_state.blind_focused_action == BLIND_ACTION_TAG)
            {
                g_game_state.blind_focused_action = BLIND_ACTION_SKIP;
                audio_play_sfx(AUDIO_SFX_HIGHLIGHT);
            }
            else if (g_game_state.blind_focused_action == BLIND_ACTION_SKIP)
            {
                g_game_state.blind_focused_action = BLIND_ACTION_TAG;
                audio_play_sfx(AUDIO_SFX_HIGHLIGHT);
            }
            // (Run Info agora e acessado pelo SELECT)
        }
    }
}

void game_input_update_cash_out(bool no_input)
{
    if (!no_input)
    {
        if (input_was_button_pressed(INPUT_BUTTON_CROSS))
        {
            g_game_state.wealth += g_game_state.cash_out_value;

            game_go_to_stage(GAME_STAGE_SHOP, GAME_SUBSTAGE_SHOP_MAIN);
        }
    }
}

void game_input_update_sort_rank(bool no_input)
{
    if (!no_input)
    {
        if (input_was_button_pressed(INPUT_BUTTON_CROSS))
        {
            g_game_state.sort_criteria = SORT_CRITERIA_RANK;
            automated_event_set(AUTOMATED_EVENT_SORT_HAND, 0);
        }
        else if (input_was_button_pressed(INPUT_BUTTON_RIGHT))
        {
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SORT_HAND_SUIT;
        }
        else if (input_was_button_pressed(INPUT_BUTTON_UP))
        {
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_HAND;
        }
        else if (input_was_button_pressed(INPUT_BUTTON_LEFT))
        {
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_HAND_PLAY;
        }
    }
}

void game_input_update_sort_suit(bool no_input)
{
    if (!no_input)
    {
        if (input_was_button_pressed(INPUT_BUTTON_CROSS))
        {
            g_game_state.sort_criteria = SORT_CRITERIA_SUIT;
            automated_event_set(AUTOMATED_EVENT_SORT_HAND, 0);
        }
        else if (input_was_button_pressed(INPUT_BUTTON_LEFT))
        {
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SORT_HAND_RANK;
        }
        else if (input_was_button_pressed(INPUT_BUTTON_UP))
        {
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_HAND;
        }
        else if (input_was_button_pressed(INPUT_BUTTON_RIGHT))
        {
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_HAND_DISCARD;
        }
    }
}

void game_input_update_shop_next_round(bool no_input)
{
    if (!no_input)
    {
        if (input_was_button_pressed(INPUT_BUTTON_CROSS))
        {
            game_go_to_stage(GAME_STAGE_BLINDS, GAME_SUBSTAGE_BLINDS_DEFAULT);
        }

        if (input_was_button_pressed(INPUT_BUTTON_DOWN))
        {
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_REROLL;
        }
        else if (input_was_button_pressed(INPUT_BUTTON_UP))
        {
            if (g_game_state.jokers.joker_count > 0)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_JOKERS;
            }
            else if (g_game_state.consumables.item_count > 0)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_CONSUMABLES;
            }
        }
        else if (input_was_button_pressed(INPUT_BUTTON_RIGHT))
        {
            if (g_game_state.shop.item_count > 0)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_SINGLES;
                g_game_state.highlighted_item = game_util_get_first_shop_single_index();
            }
            else
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_DECK;
            }
        }
        // (Run Info agora e acessado pelo SELECT: LEFT nao faz nada aqui)
    }
}

void game_input_update_shop_reroll(bool no_input)
{
    if (!no_input)
    {
        if (input_was_button_pressed(INPUT_BUTTON_UP))
        {
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_NEXT_ROUND;
        }
        else if (input_was_button_pressed(INPUT_BUTTON_DOWN))
        {
            if (g_game_state.shop.booster_count > 0)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_BOOSTERS;
                g_game_state.highlighted_item = game_util_get_first_shop_booster_index();
            }
        }
        else if (input_was_button_pressed(INPUT_BUTTON_RIGHT) && g_game_state.shop.item_count > 0)
        {
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_SINGLES;
            g_game_state.highlighted_item = game_util_get_first_shop_single_index();
        }
        // (Run Info agora e acessado pelo SELECT: LEFT nao faz nada aqui)

        if (input_was_button_pressed(INPUT_BUTTON_CROSS))
        {
            game_shop_reroll();
        }
    }
}

void game_input_update_shop_singles(bool no_input)
{
    if (!no_input)
    {
        if (input_was_button_pressed(INPUT_BUTTON_UP))
        {
            if (g_game_state.jokers.joker_count > 0)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_JOKERS;
                g_game_state.highlighted_item = 0;
            }
            else if (g_game_state.consumables.item_count > 0)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_CONSUMABLES;
                g_game_state.highlighted_item = 0;
            }
        }
        else if (input_was_button_pressed(INPUT_BUTTON_DOWN))
        {
            if (g_game_state.shop.booster_count > 0)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_BOOSTERS;
                g_game_state.highlighted_item = game_util_get_first_shop_booster_index();
            }
        }
        else if (input_was_button_pressed(INPUT_BUTTON_RIGHT))
        {        
            if (g_game_state.highlighted_item < g_game_state.shop.total_items - 1)
            {
                do
                {
                    g_game_state.highlighted_item++;
                } while (!g_game_state.shop.items[g_game_state.highlighted_item].available && g_game_state.highlighted_item < g_game_state.shop.total_items - 1);

                if (!g_game_state.shop.items[g_game_state.highlighted_item].available)
                {
                    g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_DECK;
                }
            }
            else
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_DECK;
            }
        }
        else if (input_was_button_pressed(INPUT_BUTTON_LEFT))
        {
            if (g_game_state.highlighted_item > 0)
            {
                do
                {
                    g_game_state.highlighted_item--;
                } while (!g_game_state.shop.items[g_game_state.highlighted_item].available && g_game_state.highlighted_item > 0);

                if (!g_game_state.shop.items[g_game_state.highlighted_item].available)
                {
                    g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_NEXT_ROUND;
                }
            }
            else
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_NEXT_ROUND;
            }
        }

        if (input_was_button_pressed(INPUT_BUTTON_RIGHT_TRIGGER))
        {
            game_shop_buy_highlighted_item(false);
        }

        if (input_was_button_pressed(INPUT_BUTTON_LEFT_TRIGGER))
        {
            game_shop_buy_highlighted_item(true);
        }
    }
}

void game_input_update_shop_boosters(bool no_input)
{
    if (!no_input)
    {
        if (input_was_button_pressed(INPUT_BUTTON_UP))
        {
            if (g_game_state.shop.item_count > 0)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_SINGLES;
                g_game_state.highlighted_item = game_util_get_first_shop_item_index();
            }
            else
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_NEXT_ROUND;
                g_game_state.highlighted_item = 0;
            }
        }
        else if (input_was_button_pressed(INPUT_BUTTON_RIGHT))
        {        
            if (g_game_state.highlighted_item < g_game_state.shop.total_boosters - 1)
            {
                int last_hightlighted_item = g_game_state.highlighted_item;
                do
                {
                    g_game_state.highlighted_item++;
                } while (!g_game_state.shop.boosters[g_game_state.highlighted_item].available && g_game_state.highlighted_item < g_game_state.shop.total_boosters - 1);

                if (!g_game_state.shop.boosters[g_game_state.highlighted_item].available)
                {
                    g_game_state.highlighted_item = last_hightlighted_item;
                    g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_DECK;
                }
            }
            else
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_DECK;
            }
        }
        else if (input_was_button_pressed(INPUT_BUTTON_LEFT))
        {
            if (g_game_state.highlighted_item > 0)
            {
                int last_hightlighted_item = g_game_state.highlighted_item;
                do
                {
                    g_game_state.highlighted_item--;
                } while (!g_game_state.shop.boosters[g_game_state.highlighted_item].available && g_game_state.highlighted_item > 0);

                if (!g_game_state.shop.boosters[g_game_state.highlighted_item].available)
                {
                    g_game_state.highlighted_item = last_hightlighted_item;
                    // (Run Info agora e acessado pelo SELECT)
                }
            }
            // (Run Info agora e acessado pelo SELECT: LEFT nao faz nada aqui)
        }

        if (input_was_button_pressed(INPUT_BUTTON_RIGHT_TRIGGER))
        {
            game_buy_booster();
        }
    }
}

void game_input_update_booster_items(bool no_input)
{
    if (no_input) return;

    if (input_was_button_pressed(INPUT_BUTTON_RIGHT))
    {
        if (g_game_state.highlighted_item < g_game_state.shop.booster_total_items - 1)
        {
            int last_hightlighted_item = g_game_state.highlighted_item;
            do
            {
                g_game_state.highlighted_item++;
            } while (!g_game_state.shop.booster_items[g_game_state.highlighted_item].available && g_game_state.highlighted_item < g_game_state.shop.booster_total_items - 1);

            if (!g_game_state.shop.booster_items[g_game_state.highlighted_item].available)
            {
                g_game_state.highlighted_item = last_hightlighted_item;
            }
        }
        else
        {
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_DECK;
        }
    }
    else if (input_was_button_pressed(INPUT_BUTTON_LEFT))
    {
        if (g_game_state.highlighted_item > 0)
        {
            int last_hightlighted_item = g_game_state.highlighted_item;
            do
            {
                g_game_state.highlighted_item--;
            } while (!g_game_state.shop.booster_items[g_game_state.highlighted_item].available && g_game_state.highlighted_item > 0);

            if (!g_game_state.shop.booster_items[g_game_state.highlighted_item].available)
            {
                g_game_state.highlighted_item = last_hightlighted_item;
            }
        }
        // (Run Info agora e acessado pelo SELECT: LEFT nao faz nada aqui)
    }
    else if (input_was_button_pressed(INPUT_BUTTON_UP))
    {
        if (g_game_state.sub_stage == GAME_SUBSTAGE_SHOP_BOOSTER_HAND)
        {
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_HAND;
        }
        else
        {
            if (g_game_state.jokers.joker_count > 0)
            {
                g_game_state.highlighted_item = 0;
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_JOKERS;
            }
            else if (g_game_state.consumables.item_count > 0)
            {
                g_game_state.highlighted_item = 0;
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_CONSUMABLES;
            }
        }
    }

    if (input_was_button_pressed(INPUT_BUTTON_TRIANGLE))
    {
        g_game_state.highlighted_item = 0;
        game_go_to_stage(GAME_STAGE_SHOP, GAME_SUBSTAGE_SHOP_MAIN);
    }

    if (input_was_button_pressed(INPUT_BUTTON_RIGHT_TRIGGER))
    {
        game_use_booster_item();
    }
}

void game_input_update_hand_play(bool no_input)
{
    if (no_input) return;

    if (input_was_button_pressed(INPUT_BUTTON_CROSS))
    {
        if (game_util_can_play_selected_hand())
        {
            g_game_state.highlighted_item = 0;
            automated_event_set(AUTOMATED_EVENT_SCORE, 0);
            g_game_state.current_hands--;
        }
    }
    else if (input_was_button_pressed(INPUT_BUTTON_RIGHT))
    {
        g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SORT_HAND_RANK;
    }
    else if (input_was_button_pressed(INPUT_BUTTON_UP))
    {
        g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_HAND;
    }
    // (Run Info agora e acessado pelo SELECT: LEFT nao faz nada aqui)
}

void game_input_update_hand_discard(bool no_input)
{
    if (no_input) return;

    if (input_was_button_pressed(INPUT_BUTTON_CROSS))
    {
        game_discard_selected_cards();
    }
    else if (input_was_button_pressed(INPUT_BUTTON_LEFT))
    {
        g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SORT_HAND_SUIT;
    }
    else if (input_was_button_pressed(INPUT_BUTTON_RIGHT))
    {
        g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_DECK;
    }
    else if (input_was_button_pressed(INPUT_BUTTON_UP))
    {
        g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_HAND;
    }
}

void game_input_update_you_win_panel(bool no_input)
{
    if (no_input) return;

    if (input_was_button_pressed(INPUT_BUTTON_CROSS))
    {
        game_show_title_menu();
        g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_DECK_SELECT;
        g_game_state.highlighted_item = CLAMP(g_game_state.deck_type, 0, DECK_TYPE_COUNT - 1);
    }
}

void game_input_update_game_over_panel(bool no_input)
{
    if (no_input) return;

    if (input_was_button_pressed(INPUT_BUTTON_CROSS))
    {
        game_show_title_menu();
        g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_DECK_SELECT;
        g_game_state.highlighted_item = CLAMP(g_game_state.deck_type, 0, DECK_TYPE_COUNT - 1);
    }
}

void game_input_update_deck(bool no_input)
{
    if (no_input) return;

    if (input_was_button_pressed(INPUT_BUTTON_CROSS))
    {        
        game_go_to_stage(g_game_state.stage, GAME_SUBSTAGE_DECK_INFO);
    }

    if (input_was_button_pressed(INPUT_BUTTON_LEFT))
    {
        switch(g_game_state.stage)
        {
            case GAME_STAGE_BLINDS:
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_BLIND;
                break;
            }
            case GAME_STAGE_INGAME:
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_HAND;
                break;
            }
            case GAME_STAGE_SHOP:
            {
                if (g_game_state.sub_stage == GAME_SUBSTAGE_SHOP_BOOSTER_HAND ||
                    g_game_state.sub_stage == GAME_SUBSTAGE_SHOP_BOOSTER_NO_HAND)
                {
                    g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_BOOSTER_ITEMS;
                    g_game_state.highlighted_item = game_util_get_first_shop_booster_item_index();
                }
                else
                {
                    if (g_game_state.shop.booster_count > 0)
                    {
                        g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_BOOSTERS;
                        g_game_state.highlighted_item = game_util_get_first_shop_booster_index();
                    }
                    else if (g_game_state.shop.item_count > 0)
                    {
                        g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_SINGLES;
                        g_game_state.highlighted_item = game_util_get_first_shop_single_index();
                    }
                    else
                    {
                        g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_NEXT_ROUND;
                    }
                }
            }
        }
    }
}

void game_input_update_deck_info_close(bool no_input)
{
    if (no_input) return;

    if (input_was_button_pressed(INPUT_BUTTON_CROSS) ||
        input_was_button_pressed(INPUT_BUTTON_CIRCLE))
    {
        game_go_back_to_previous_stage();
    }
    else if (input_was_button_pressed(INPUT_BUTTON_UP))
    {
        game_util_get_first_deck_info_card(&g_game_state.deck_info.current_card_x, &g_game_state.deck_info.current_card_y);
        if (g_game_state.deck_info.current_card_x > -1 && g_game_state.deck_info.current_card_y > -1)
        {
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_DECK_INFO_CARD;
            g_game_state.deck_info.current_card = *(g_game_state.deck_info.cards[g_game_state.deck_info.current_card_y][g_game_state.deck_info.current_card_x]);
            g_game_state.deck_info.current_card.draw = g_game_state.deck_info.draw[g_game_state.deck_info.current_card_y][g_game_state.deck_info.current_card_x];
        }
    }
}

void game_input_update_deck_info_card(bool no_input)
{
    if (no_input) return;

    if (input_was_button_pressed(INPUT_BUTTON_CIRCLE))
    {
        g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_DECK_INFO_CLOSE;
        return;
    }

    if (input_was_button_pressed(INPUT_BUTTON_LEFT))
    {
        if (g_game_state.deck_info.current_card_x > 0)
        {
            g_game_state.deck_info.current_card_x--;
        }
        else
        {
            g_game_state.deck_info.current_card_x = g_game_state.deck_info.card_count[g_game_state.deck_info.current_card_y] - 1;
        }

        g_game_state.deck_info.current_card = *(g_game_state.deck_info.cards[g_game_state.deck_info.current_card_y][g_game_state.deck_info.current_card_x]);
        g_game_state.deck_info.current_card.draw = g_game_state.deck_info.draw[g_game_state.deck_info.current_card_y][g_game_state.deck_info.current_card_x];
    }
    else if (input_was_button_pressed(INPUT_BUTTON_RIGHT))
    {
        if (g_game_state.deck_info.current_card_x < g_game_state.deck_info.card_count[g_game_state.deck_info.current_card_y] - 1)
        {
            g_game_state.deck_info.current_card_x++;
        }
        else
        {
            g_game_state.deck_info.current_card_x = 0;
        }

        g_game_state.deck_info.current_card = *(g_game_state.deck_info.cards[g_game_state.deck_info.current_card_y][g_game_state.deck_info.current_card_x]);
        g_game_state.deck_info.current_card.draw = g_game_state.deck_info.draw[g_game_state.deck_info.current_card_y][g_game_state.deck_info.current_card_x];
    }
    else if (input_was_button_pressed(INPUT_BUTTON_UP))
    {
        int y = g_game_state.deck_info.current_card_y;
        do
        {
            g_game_state.deck_info.current_card_y--;
            if (g_game_state.deck_info.current_card_y < 0)
            {
                g_game_state.deck_info.current_card_y = 3;
            }
        } while (g_game_state.deck_info.card_count[g_game_state.deck_info.current_card_y] == 0);

        if (y != g_game_state.deck_info.current_card_y)
        {
            g_game_state.deck_info.current_card_x = MIN(g_game_state.deck_info.current_card_x, g_game_state.deck_info.card_count[g_game_state.deck_info.current_card_y] - 1);
            g_game_state.deck_info.current_card = *(g_game_state.deck_info.cards[g_game_state.deck_info.current_card_y][g_game_state.deck_info.current_card_x]);
            g_game_state.deck_info.current_card.draw = g_game_state.deck_info.draw[g_game_state.deck_info.current_card_y][g_game_state.deck_info.current_card_x];
        }
    }
    else if (input_was_button_pressed(INPUT_BUTTON_DOWN))
    {
        int y = g_game_state.deck_info.current_card_y;
        do
        {
            g_game_state.deck_info.current_card_y++;
            if (g_game_state.deck_info.current_card_y > 3)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_DECK_INFO_CLOSE;
                return;
            }
        } while (g_game_state.deck_info.card_count[g_game_state.deck_info.current_card_y] == 0);
        
        if (y != g_game_state.deck_info.current_card_y)
        {
            g_game_state.deck_info.current_card_x = MIN(g_game_state.deck_info.current_card_x, g_game_state.deck_info.card_count[g_game_state.deck_info.current_card_y] - 1);
            g_game_state.deck_info.current_card = *(g_game_state.deck_info.cards[g_game_state.deck_info.current_card_y][g_game_state.deck_info.current_card_x]);
            g_game_state.deck_info.current_card.draw = g_game_state.deck_info.draw[g_game_state.deck_info.current_card_y][g_game_state.deck_info.current_card_x];
        }
    }
}

void game_input_update_deck_info(bool no_input)
{
    if (no_input) return;

    if (input_was_button_pressed(INPUT_BUTTON_LEFT_TRIGGER))
    {
        g_game_state.deck_info.partial = !g_game_state.deck_info.partial;
        game_draw_offscreen_deck_info();
    }
    else if (input_was_button_pressed(INPUT_BUTTON_RIGHT_TRIGGER))
    {
        g_game_state.deck_info.effective = !g_game_state.deck_info.effective;
    }
}

void game_input_update_run_info(bool no_input)
{
    if (no_input) return;

    if (input_was_button_pressed(INPUT_BUTTON_CROSS) ||
        input_was_button_pressed(INPUT_BUTTON_SELECT))
    {
        // Nunca reempilhar o overlay sobre ele mesmo (duplo SELECT travava):
        // aqui sempre fecha.
        game_go_back_to_previous_stage();
    }
    else if (input_was_button_pressed(INPUT_BUTTON_RIGHT))
    {
        switch (g_game_state.stage)
        {
            case GAME_STAGE_BLINDS:
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_BLIND;
                break;
            case GAME_STAGE_SHOP:
                if (g_game_state.sub_stage == GAME_SUBSTAGE_SHOP_MAIN)
                {
                    if (g_game_state.shop.booster_count > 0)
                    {
                        g_game_state.highlighted_item = game_util_get_first_shop_booster_index();
                        g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_BOOSTERS;
                    }
                    else
                    {
                        g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_NEXT_ROUND;
                    }
                }
                else
                {
                    g_game_state.highlighted_item = game_util_get_first_shop_booster_item_index();
                    g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_BOOSTER_ITEMS;
                }
                break;
            case GAME_STAGE_INGAME:
                g_game_state.highlighted_item = 0;
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_HAND;
                break;
        }
    }
}

void game_input_update_options(bool no_input)
{
    if (no_input) return;

    if (input_was_button_pressed(INPUT_BUTTON_CROSS))
    {
        game_go_to_stage(GAME_STAGE_PAUSE_MENU, 0);
        g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SETTINGS_MENU;
        g_game_state.highlighted_item = 0;
    }
    else if (input_was_button_pressed(INPUT_BUTTON_SELECT))
    {
        if (g_game_state.sub_stage != GAME_SUBSTAGE_RUN_INFO &&
            g_game_state.sub_stage != GAME_SUBSTAGE_DECK_INFO)
        {
            game_go_to_stage(g_game_state.stage, GAME_SUBSTAGE_RUN_INFO);
        }
    }
    else if (input_was_button_pressed(INPUT_BUTTON_RIGHT))
    {
        switch (g_game_state.stage)
        {
            case GAME_STAGE_BLINDS:
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_BLIND;
                break;
            case GAME_STAGE_SHOP:
                if (g_game_state.sub_stage == GAME_SUBSTAGE_SHOP_MAIN)
                {
                    if (g_game_state.shop.booster_count > 0)
                    {
                        g_game_state.highlighted_item = game_util_get_first_shop_booster_index();
                        g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_BOOSTERS;
                    }
                    else
                    {
                        g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_NEXT_ROUND;
                    }
                }
                else
                {
                    g_game_state.highlighted_item = game_util_get_first_shop_booster_item_index();
                    g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_BOOSTER_ITEMS;
                }
                break;
            case GAME_STAGE_INGAME:
                g_game_state.highlighted_item = 0;
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_HAND;
                break;
        }
    }
}

void game_input_update_pause_menu(bool no_input)
{
    if (no_input) return;

    if (input_was_button_pressed(INPUT_BUTTON_DOWN))
    {
        g_game_state.highlighted_item = (g_game_state.highlighted_item + 1) % 5;
    }
    else if (input_was_button_pressed(INPUT_BUTTON_UP))
    {
        if (g_game_state.highlighted_item == 0)
            g_game_state.highlighted_item = 4;
        else
            g_game_state.highlighted_item--;
    }

    if (input_was_button_pressed(INPUT_BUTTON_CROSS))
    {
        if (g_game_state.highlighted_item == 0) // Continue
        {
            game_go_back_to_previous_stage();
        }
        else if (g_game_state.highlighted_item == 1) // Save
        {
            game_go_back_to_previous_stage();
            run_save_utility();
        }
        else if (g_game_state.highlighted_item == 2) // Load
        {
            if (run_load_utility())
            {
                event_init();
            }
            else
            {
                game_go_back_to_previous_stage();
            }
        }
        else if (g_game_state.highlighted_item == 3) // Settings
        {
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SETTINGS_MENU;
            g_game_state.highlighted_item = 0;
        }
        else if (g_game_state.highlighted_item == 4) // Main Menu
        {
            game_go_back_to_previous_stage();
            save_write_autosave();
            game_show_title_menu();
        }
    }
    else if (input_was_button_pressed(INPUT_BUTTON_START) || input_was_button_pressed(INPUT_BUTTON_CIRCLE))
    {
        game_go_back_to_previous_stage();
    }
}

static const int g_title_menu_order[TITLE_MENU_ITEM_COUNT] = {
    TITLE_MENU_ITEM_LOAD,
    TITLE_MENU_ITEM_PLAY,
    TITLE_MENU_ITEM_OPTIONS,
    TITLE_MENU_ITEM_QUIT,
    TITLE_MENU_ITEM_CONTINUE
};

static void game_input_title_move(int direction)
{
    int highlighted = g_game_state.highlighted_item;
    int index = 1; // fallback: PLAY

    for (int i = 0; i < TITLE_MENU_ITEM_COUNT; i++)
    {
        if (g_title_menu_order[i] == highlighted)
        {
            index = i;
            break;
        }
    }

    for (int i = 0; i < TITLE_MENU_ITEM_COUNT; i++)
    {
        index = (index + direction + TITLE_MENU_ITEM_COUNT) % TITLE_MENU_ITEM_COUNT;
        int item = g_title_menu_order[index];
        if (item != TITLE_MENU_ITEM_CONTINUE || save_autosave_exists())
        {
            g_game_state.highlighted_item = item;
            break;
        }
    }
}

void game_input_update_title_menu(bool no_input)
{
    if (no_input) return;

    if (g_game_state.highlighted_item < 0 || g_game_state.highlighted_item >= TITLE_MENU_ITEM_COUNT)
    {
        g_game_state.highlighted_item = TITLE_MENU_ITEM_PLAY;
    }
    if (g_game_state.highlighted_item == TITLE_MENU_ITEM_CONTINUE && !save_autosave_exists())
    {
        g_game_state.highlighted_item = TITLE_MENU_ITEM_PLAY;
    }

    if (input_was_button_pressed(INPUT_BUTTON_LEFT) || input_was_button_pressed(INPUT_BUTTON_UP))
    {
        game_input_title_move(-1);
    }
    else if (input_was_button_pressed(INPUT_BUTTON_RIGHT) || input_was_button_pressed(INPUT_BUTTON_DOWN))
    {
        game_input_title_move(1);
    }

    if (input_was_button_pressed(INPUT_BUTTON_CROSS) ||
        input_was_button_pressed(INPUT_BUTTON_START))
    {
        if (g_game_state.highlighted_item == TITLE_MENU_ITEM_CONTINUE)
        {
            if (save_load_autosave())
            {
                event_init();
            }
            else
            {
                g_game_state.highlighted_item = TITLE_MENU_ITEM_PLAY;
            }
        }
        else if (g_game_state.highlighted_item == TITLE_MENU_ITEM_PLAY)
        {
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_DECK_SELECT;
            g_game_state.highlighted_item = DECK_TYPE_RED;
        }
        else if (g_game_state.highlighted_item == TITLE_MENU_ITEM_OPTIONS)
        {
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SETTINGS_MENU;
            g_game_state.highlighted_item = 0;
        }
        else if (g_game_state.highlighted_item == TITLE_MENU_ITEM_QUIT)
        {
            request_exit();
        }
        else if (g_game_state.highlighted_item == TITLE_MENU_ITEM_LOAD)
        {
            if (run_load_utility())
            {
                event_init();
            }
            else
            {
                game_show_title_menu();
            }
        }
    }
    else if (input_was_button_pressed(INPUT_BUTTON_CIRCLE))
    {
        request_exit();
    }
}

void game_input_update_deck_select(bool no_input)
{
    if (no_input) return;

    if (input_was_button_pressed(INPUT_BUTTON_RIGHT))
    {
        g_game_state.highlighted_item = (g_game_state.highlighted_item + 1) % DECK_TYPE_COUNT;
    }
    else if (input_was_button_pressed(INPUT_BUTTON_LEFT))
    {
        g_game_state.highlighted_item = (g_game_state.highlighted_item + DECK_TYPE_COUNT - 1) % DECK_TYPE_COUNT;
    }
    else if (input_was_button_pressed(INPUT_BUTTON_DOWN))
    {
        g_game_state.highlighted_item = (g_game_state.highlighted_item + 3) % DECK_TYPE_COUNT;
    }
    else if (input_was_button_pressed(INPUT_BUTTON_UP))
    {
        g_game_state.highlighted_item = (g_game_state.highlighted_item + DECK_TYPE_COUNT - 3) % DECK_TYPE_COUNT;
    }

    if (input_was_button_pressed(INPUT_BUTTON_CROSS))
    {
        bool unlocked = profile_is_deck_unlocked(g_game_state.highlighted_item);
        if (unlocked)
        {
            game_start_new_run(g_game_state.highlighted_item);
        }
    }
    else if (input_was_button_pressed(INPUT_BUTTON_CIRCLE))
    {
        g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_TITLE_MENU;
        g_game_state.highlighted_item = TITLE_MENU_ITEM_PLAY;
    }
}

void game_input_apply_overclock_setting()
{
    if (g_settings.overclock)
    {
        scePowerSetClockFrequency(333, 333, 166);
    }
    else
    {
        scePowerSetClockFrequency(222, 222, 111);
    }
}

void game_input_update_settings_value(int direction)
{
    switch (g_game_state.highlighted_item)
    {
        case 0:
            g_settings.audio = !g_settings.audio;
            break;
        case 1:
            g_settings.music_volume = CLAMP(g_settings.music_volume + direction, 0, 10);
            break;
        case 2:
            g_settings.sfx_volume = CLAMP(g_settings.sfx_volume + direction, 0, 10);
            break;
        case 3:
            g_settings.move_cards = !g_settings.move_cards;
            break;
        case 4:
            g_settings.debug_tools = !g_settings.debug_tools;
            if (!g_settings.debug_tools)
            {
                g_debug_info.force_score_flames = false;
            }
            break;
        case 5:
            g_settings.speed = CLAMP(g_settings.speed + direction, 1, 5);
            break;
        case 6:
            g_settings.ante_score_scaling = CLAMP(g_settings.ante_score_scaling + direction, 1, 3);
            g_game_state.ante_score_scaling = g_settings.ante_score_scaling - 1;
            break;
        case 7:
            g_settings.overclock = !g_settings.overclock;
            game_input_apply_overclock_setting();
            break;
        case 8:
            if (g_game_state.stage == GAME_STAGE_TITLE)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_TITLE_MENU;
                g_game_state.highlighted_item = TITLE_MENU_ITEM_OPTIONS;
            }
            else
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_PAUSE_MENU;
                g_game_state.highlighted_item = 3;
            }
            break;
    }
}

void game_input_update_settings_menu(bool no_input)
{
    if (no_input) return;

    if (input_was_button_pressed(INPUT_BUTTON_DOWN))
    {
        g_game_state.highlighted_item = (g_game_state.highlighted_item + 1) % 9;
    }
    else if (input_was_button_pressed(INPUT_BUTTON_UP))
    {
        if (g_game_state.highlighted_item == 0)
            g_game_state.highlighted_item = 8;
        else
            g_game_state.highlighted_item--;
    }

    if (input_was_button_pressed(INPUT_BUTTON_LEFT))
    {
        game_input_update_settings_value(-1);
    }
    else if (input_was_button_pressed(INPUT_BUTTON_RIGHT))
    {
        game_input_update_settings_value(1);
    }
    else if (input_was_button_pressed(INPUT_BUTTON_CROSS))
    {
        game_input_update_settings_value(1);
    }
    else if (input_was_button_pressed(INPUT_BUTTON_CIRCLE))
    {
        if (g_game_state.stage == GAME_STAGE_TITLE)
        {
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_TITLE_MENU;
            g_game_state.highlighted_item = TITLE_MENU_ITEM_OPTIONS;
        }
        else
        {
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_PAUSE_MENU;
            g_game_state.highlighted_item = 3;
        }
    }
    else if (input_was_button_pressed(INPUT_BUTTON_START))
    {
        if (g_game_state.stage == GAME_STAGE_TITLE)
        {
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_TITLE_MENU;
            g_game_state.highlighted_item = TITLE_MENU_ITEM_OPTIONS;
        }
        else
        {
            game_go_back_to_previous_stage();
        }
    }
}

void game_input_update_run_info_close(bool no_input)
{
    if (no_input) return;

    if (input_was_button_pressed(INPUT_BUTTON_CROSS) ||
        input_was_button_pressed(INPUT_BUTTON_CIRCLE) ||
        input_was_button_pressed(INPUT_BUTTON_SELECT))
    {
        game_go_back_to_previous_stage();
    }
}

bool game_input_debug_update_seal()
{
    if (!g_settings.debug_tools) return false;

    if (g_game_state.stage != GAME_STAGE_INGAME ||
        g_game_state.sub_stage != GAME_SUBSTAGE_INGAME_PICK_HAND ||
        g_game_state.hand.card_count <= 0)
    {
        return false;
    }

    if (g_game_state.input_focused_zone != INPUT_FOCUSED_ZONE_HAND)
    {
        g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_HAND;
        g_game_state.highlighted_item = 0;
    }

    if (g_game_state.highlighted_item < 0 || g_game_state.highlighted_item >= g_game_state.hand.card_count)
    {
        g_game_state.highlighted_item = 0;
    }

    struct Card *card = g_game_state.hand.cards[g_game_state.highlighted_item];
    if (input_was_button_pressed(INPUT_BUTTON_SQUARE))
    {
        card->seal = (card->seal + 1) % CARD_SEAL_COUNT;
    }
    else if (input_was_button_pressed(INPUT_BUTTON_CIRCLE))
    {
        card->seal = CARD_SEAL_NONE;
    }
    else
    {
        return false;
    }

    event_add_pop_item(&(card->draw), 18);
    event_add_shake_item(&(card->draw), 20);
    audio_play_sfx(AUDIO_SFX_BUTTON);
    return true;
}

bool game_input_debug_update_edition()
{
    if (!g_settings.debug_tools) return false;

    if (g_game_state.stage != GAME_STAGE_INGAME ||
        g_game_state.sub_stage != GAME_SUBSTAGE_INGAME_PICK_HAND ||
        g_game_state.hand.card_count <= 0)
    {
        return false;
    }

    if (g_game_state.input_focused_zone != INPUT_FOCUSED_ZONE_HAND)
    {
        g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_HAND;
        g_game_state.highlighted_item = 0;
    }

    if (g_game_state.highlighted_item < 0 || g_game_state.highlighted_item >= g_game_state.hand.card_count)
    {
        g_game_state.highlighted_item = 0;
    }

    struct Card *card = g_game_state.hand.cards[g_game_state.highlighted_item];
    if (input_was_button_pressed(INPUT_BUTTON_CROSS))
    {
        card->edition = (card->edition + 1) % CARD_EDITION_NEGATIVE;
    }
    else
    {
        return false;
    }

    event_add_pop_item(&(card->draw), 18);
    event_add_shake_item(&(card->draw), 20);
    audio_play_sfx(AUDIO_SFX_BUTTON);
    return true;
}

bool game_input_debug_update_decks()
{
    if (!g_settings.debug_tools) return false;
    if (g_game_state.stage != GAME_STAGE_TITLE ||
        g_game_state.input_focused_zone != INPUT_FOCUSED_ZONE_DECK_SELECT)
    {
        return false;
    }

    if (input_was_button_pressed(INPUT_BUTTON_CROSS))
    {
        profile_debug_unlock_next_deck_tier();
    }
    else if (input_was_button_pressed(INPUT_BUTTON_CIRCLE))
    {
        profile_debug_lock_all_progress();
        g_game_state.highlighted_item = DECK_TYPE_RED;
    }
    else
    {
        return false;
    }

    audio_play_sfx(AUDIO_SFX_BUTTON);
    return true;
}

bool game_input_debug_update_boss_blind()
{
    if (!g_settings.debug_tools) return false;
    if (g_game_state.stage != GAME_STAGE_BLINDS)
    {
        return false;
    }

    if (input_was_button_pressed(INPUT_BUTTON_START))
    {
        g_game_state.blind = GAME_BLIND_BOSS;
        g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_BLIND;

        bool showdown = g_game_state.ante > 0 && g_game_state.ante % 8 == 0;
        do
        {
            g_game_state.boss_blind_type = (g_game_state.boss_blind_type + 1) % BOSS_BLIND_COUNT;
        } while (g_boss_blind_types[g_game_state.boss_blind_type].showdown != showdown ||
                 g_game_state.ante < g_boss_blind_types[g_game_state.boss_blind_type].min_ante);

        audio_play_sfx(AUDIO_SFX_BUTTON);
        return true;
    }

    return false;
}

bool game_input_debug_update_spectral_boosters()
{
    if (!g_settings.debug_tools) return false;
    if (g_game_state.stage != GAME_STAGE_SHOP ||
        g_game_state.sub_stage != GAME_SUBSTAGE_SHOP_MAIN)
    {
        return false;
    }

    if (input_was_button_pressed(INPUT_BUTTON_START))
    {
        for (int i = 0; i < g_game_state.shop.total_boosters; i++)
        {
            g_game_state.shop.boosters[i].available = true;
            g_game_state.shop.boosters[i].booster.type = BOOSTER_PACK_TYPE_SPECTRAL;
            g_game_state.shop.boosters[i].booster.size = BOOSTER_PACK_SIZE_MEGA;
            g_game_state.shop.boosters[i].booster.image = i % BOOSTER_PACK_MAX_IMAGES;
        }
        g_game_state.shop.booster_count = g_game_state.shop.total_boosters;
        game_set_shop_booster_position();
        g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_BOOSTERS;
        g_game_state.highlighted_item = game_util_get_first_shop_booster_index();

        audio_play_sfx(AUDIO_SFX_BUTTON);
        return true;
    }

    return false;
}

void game_input_update(bool no_input)
{
    if (g_settings.debug_tools &&
        input_is_button_down(INPUT_BUTTON_LEFT_TRIGGER) &&
        input_is_button_down(INPUT_BUTTON_RIGHT_TRIGGER) &&
        input_was_button_pressed(INPUT_BUTTON_TRIANGLE))
    {
        g_debug_info.force_score_flames = !g_debug_info.force_score_flames;
        audio_play_sfx(AUDIO_SFX_BUTTON);
        return;
    }

    if (input_is_button_down(INPUT_BUTTON_LEFT_TRIGGER) &&
        input_is_button_down(INPUT_BUTTON_RIGHT_TRIGGER) &&
        (game_input_debug_update_decks() || game_input_debug_update_seal() || game_input_debug_update_edition()
         || game_input_debug_update_boss_blind() || game_input_debug_update_spectral_boosters()
        ))
    {
        return;
    }

    // Estado explicito do modal Run Info: SELECT alterna abrir/fechar.
    // Nao usa previous_stage (pilha de 1 nivel corrompia com 2x SELECT,
    // START por cima, evento no meio, etc). Abrir salva o retorno aqui,
    // fechar restaura daqui. Simples.
    static bool s_run_info_open = false;
    static int s_run_info_stage = -1;
    static int s_run_info_sub = -1;
    static int s_run_info_focus = -1;

    bool in_game_stage = (g_game_state.stage == GAME_STAGE_BLINDS ||
                          g_game_state.stage == GAME_STAGE_INGAME ||
                          g_game_state.stage == GAME_STAGE_SHOP);
    bool is_run_info = in_game_stage &&
                       g_game_state.sub_stage == GAME_SUBSTAGE_RUN_INFO;

    // Se algo externo trocou o sub por baixo, ressincroniza o flag.
    if (s_run_info_open && !is_run_info)
    {
        s_run_info_open = false;
    }

    // Modal aberto: SELECT fecha, CROSS/CIRCLE fecha, resto engolido
    // (inclusive START, para nao empilhar Pause sobre o modal).
    if (in_game_stage && (s_run_info_open || is_run_info))
    {
        if (input_was_button_pressed(INPUT_BUTTON_SELECT))
        {
            audio_play_sfx(AUDIO_SFX_BUTTON);
            if (s_run_info_open)
            {
                g_game_state.stage = s_run_info_stage;
                g_game_state.sub_stage = s_run_info_sub;
                g_game_state.input_focused_zone = s_run_info_focus;
                s_run_info_open = false;
            }
            else
            {
                // Caminho legado (aberto sem passar pelo estado): fecha 1x.
                game_go_back_to_previous_stage();
            }
            return;
        }
        if (input_was_button_pressed(INPUT_BUTTON_CROSS) ||
            input_was_button_pressed(INPUT_BUTTON_CIRCLE))
        {
            audio_play_sfx(AUDIO_SFX_BUTTON);
            if (s_run_info_open)
            {
                g_game_state.stage = s_run_info_stage;
                g_game_state.sub_stage = s_run_info_sub;
                g_game_state.input_focused_zone = s_run_info_focus;
                s_run_info_open = false;
            }
            else
            {
                game_go_back_to_previous_stage();
            }
            return;
        }
        return;
    }

    // Modal fechado: SELECT abre (so com input livre e fora do Deck Info).
    if (in_game_stage)
    {
        if (input_was_button_pressed(INPUT_BUTTON_SELECT))
        {
            if (!no_input &&
                g_game_state.sub_stage != GAME_SUBSTAGE_DECK_INFO &&
                g_game_state.sub_stage != GAME_SUBSTAGE_RUN_INFO)
            {
                s_run_info_stage = g_game_state.stage;
                s_run_info_sub = g_game_state.sub_stage;
                s_run_info_focus = g_game_state.input_focused_zone;
                g_game_state.sub_stage = GAME_SUBSTAGE_RUN_INFO;
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_RUN_INFO_CLOSE;
                s_run_info_open = true;
                audio_play_sfx(AUDIO_SFX_BUTTON);
            }
            return;
        }
    }

    // Start Tusu ile Pause (no_input dahi olsa eger kart vs. animasyonu yoksa)
    if (g_game_state.stage != GAME_STAGE_TITLE &&
        g_game_state.input_focused_zone != INPUT_FOCUSED_ZONE_PAUSE_MENU &&
        g_game_state.stage != GAME_STAGE_PAUSE_MENU)
    {
        if (input_was_button_pressed(INPUT_BUTTON_START))
        {
            audio_play_sfx(AUDIO_SFX_BUTTON);
            game_go_to_stage(GAME_STAGE_PAUSE_MENU, 0);
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SETTINGS_MENU;
            g_game_state.highlighted_item = 0;
            return;
        }
    }

    // (Modal Run Info tratado acima com estado explicito.)

    // Overlay Deck Info: self-heal do foco (automated_event pode sobrescrever
    // atras do overlay) + Close funciona mesmo com no_input.
    if (g_game_state.sub_stage == GAME_SUBSTAGE_DECK_INFO)
    {
        if (g_game_state.input_focused_zone != INPUT_FOCUSED_ZONE_DECK_INFO_CLOSE &&
            g_game_state.input_focused_zone != INPUT_FOCUSED_ZONE_DECK_INFO_CARD)
        {
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_DECK_INFO_CLOSE;
        }
        if (g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_DECK_INFO_CARD &&
            input_was_button_pressed(INPUT_BUTTON_CIRCLE))
        {
            audio_play_sfx(AUDIO_SFX_BUTTON);
            g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_DECK_INFO_CLOSE;
            return;
        }
        if (g_game_state.input_focused_zone == INPUT_FOCUSED_ZONE_DECK_INFO_CLOSE &&
            (input_was_button_pressed(INPUT_BUTTON_CROSS) ||
             input_was_button_pressed(INPUT_BUTTON_CIRCLE)))
        {
            audio_play_sfx(AUDIO_SFX_BUTTON);
            game_go_back_to_previous_stage();
            return;
        }
    }

    if (no_input) return;

    game_input_play_button_sfx();

    switch(g_game_state.input_focused_zone)
    {
        case INPUT_FOCUSED_ZONE_TITLE_MENU:
            game_input_update_title_menu(no_input);
            break;
        case INPUT_FOCUSED_ZONE_DECK_SELECT:
            game_input_update_deck_select(no_input);
            break;
        case INPUT_FOCUSED_ZONE_HAND:
            game_input_update_hand(no_input);
            break;
        case INPUT_FOCUSED_ZONE_JOKERS:
            game_input_update_joker(no_input);
            break;
        case INPUT_FOCUSED_ZONE_BLIND:
            game_input_update_blind(no_input);
            break;
        case INPUT_FOCUSED_ZONE_CASH_OUT_PANEL:
            game_input_update_cash_out(no_input);
            break;
        case INPUT_FOCUSED_ZONE_SORT_HAND_RANK:
            game_input_update_sort_rank(no_input);
            break;
        case INPUT_FOCUSED_ZONE_SORT_HAND_SUIT:
            game_input_update_sort_suit(no_input);
            break;
        case INPUT_FOCUSED_ZONE_HAND_PLAY:
            game_input_update_hand_play(no_input);
            break;
        case INPUT_FOCUSED_ZONE_HAND_DISCARD:
            game_input_update_hand_discard(no_input);
            break;
        case INPUT_FOCUSED_ZONE_SHOP_NEXT_ROUND:
            game_input_update_shop_next_round(no_input);
            break;
        case INPUT_FOCUSED_ZONE_SHOP_REROLL:
            game_input_update_shop_reroll(no_input);
            break;
        case INPUT_FOCUSED_ZONE_SHOP_SINGLES:
            game_input_update_shop_singles(no_input);
            break;
        case INPUT_FOCUSED_ZONE_SHOP_BOOSTERS:
            game_input_update_shop_boosters(no_input);
            break;
        case INPUT_FOCUSED_ZONE_CONSUMABLES:
            game_input_update_consumables(no_input);
            break;
        case INPUT_FOCUSED_ZONE_BOOSTER_ITEMS:
            game_input_update_booster_items(no_input);
            break;
        case INPUT_FOCUSED_ZONE_YOU_WIN_PANEL:
            game_input_update_you_win_panel(no_input);
            break;
        case INPUT_FOCUSED_ZONE_GAME_OVER_PANEL:
            game_input_update_game_over_panel(no_input);
            break;
        case INPUT_FOCUSED_ZONE_DECK:
            game_input_update_deck(no_input);
            break;
        case INPUT_FOCUSED_ZONE_DECK_INFO_CLOSE:
            game_input_update_deck_info_close(no_input);
            break;
        case INPUT_FOCUSED_ZONE_DECK_INFO_CARD:
            game_input_update_deck_info_card(no_input);
            break;
        case INPUT_FOCUSED_ZONE_RUN_INFO:
            game_input_update_run_info(no_input);
            break;
        case INPUT_FOCUSED_ZONE_OPTIONS:
            game_input_update_options(no_input);
            break;
        case INPUT_FOCUSED_ZONE_RUN_INFO_CLOSE:
            game_input_update_run_info_close(no_input);
            break;
        case INPUT_FOCUSED_ZONE_PAUSE_MENU:
            game_input_update_pause_menu(no_input);
            break;
        case INPUT_FOCUSED_ZONE_SETTINGS_MENU:
            game_input_update_settings_menu(no_input);
            break;
        default:
            // Foco invalido (save antigo/corrompido): sem isso o dispatch
            // nao chama handler nenhum e a tela fica "viva" mas morta.
            if (g_game_state.stage == GAME_STAGE_BLINDS)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_BLIND;
                g_game_state.blind_focused_action = BLIND_ACTION_SELECT;
                game_input_update_blind(no_input);
            }
            else if (g_game_state.stage == GAME_STAGE_INGAME)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_HAND;
                g_game_state.highlighted_item = 0;
                game_input_update_hand(no_input);
            }
            else if (g_game_state.stage == GAME_STAGE_SHOP)
            {
                if (g_game_state.sub_stage == GAME_SUBSTAGE_SHOP_BOOSTER_HAND ||
                    g_game_state.sub_stage == GAME_SUBSTAGE_SHOP_BOOSTER_NO_HAND)
                {
                    g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_BOOSTER_ITEMS;
                    g_game_state.highlighted_item = game_util_get_first_shop_booster_item_index();
                }
                else if (g_game_state.shop.booster_count > 0)
                {
                    g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_BOOSTERS;
                    g_game_state.highlighted_item = game_util_get_first_shop_booster_index();
                }
                else if (g_game_state.shop.item_count > 0)
                {
                    g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_SINGLES;
                    g_game_state.highlighted_item = game_util_get_first_shop_single_index();
                }
                else
                {
                    g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SHOP_NEXT_ROUND;
                    g_game_state.highlighted_item = 0;
                }
            }
            else if (g_game_state.stage == GAME_STAGE_TITLE)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_TITLE_MENU;
                game_input_update_title_menu(no_input);
            }
            else if (g_game_state.stage == GAME_STAGE_PAUSE_MENU)
            {
                g_game_state.input_focused_zone = INPUT_FOCUSED_ZONE_SETTINGS_MENU;
                g_game_state.highlighted_item = 0;
                game_input_update_settings_menu(no_input);
            }
            break;
    }

    if (g_game_state.sub_stage == GAME_SUBSTAGE_DECK_INFO)
    {
        game_input_update_deck_info(no_input);
    }
}
