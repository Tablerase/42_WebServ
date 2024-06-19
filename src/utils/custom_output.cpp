/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   custom_outputs.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcutte <rcutte@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 14:26:06 by rcutte            #+#    #+#             */
/*   Updated: 2024/06/14 14:29:10 by rcutte           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"
#include "color.h"

#include <iostream>
#include <iomanip>

/**
 * @brief places a separator line
*/
void SeparatorLine(){
  std::cout
    << BLU
    << " ────────────────────────────────────────────────────────── "
    << RESET << "\n";
}

/**
 * @brief places a separator block with a message in the middle
 * @param msg the message to be placed in the separator
*/
void SeparatorMsg(std::string const &msg){
  int msg_len = msg.length();
  int inter_space = (63 - msg_len) / 2;
  std::cout
    << "╔" << "══════════════════════════════════════════════════════════"
    << "╗" << "\n"
    << "║" << std::setw(inter_space + 1) << std::setfill(' ') << RESET
    << msg ;
  if (msg_len % 2 == 0)
    std::cout << " ";
  std::cout
    << std::setw(inter_space + 1) << "║" << "\n"
    << "╚" << "══════════════════════════════════════════════════════════"
    << "╝" << "\n";
}