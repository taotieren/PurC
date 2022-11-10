/*
** Copyright (C) 2022 FMSoft <https://www.fmsoft.cn>
**
** This file is a part of PurC (short for Purring Cat), an HVML interpreter.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "purc/purc.h"
#include "private/utils.h"
#include "../helpers.h"

#include <gtest/gtest.h>


static const char *calculator_1 =
    "<!DOCTYPE hvml>"
    "<hvml target=\"html\" lang=\"en\">"
    "    <head>"
    "        <title>计算器</title>"
    "        <link rel=\"stylesheet\" type=\"text/css\" href=\"calculator.css\" />"
    ""
    "        <init as=\"buttons\" uniquely>"
    "            ["
    "                { \"letters\": \"7\", \"class\": \"number\" },"
    "                { \"letters\": \"8\", \"class\": \"number\" },"
    "                { \"letters\": \"9\", \"class\": \"number\" },"
    "                { \"letters\": \"←\", \"class\": \"c_blue backspace\" },"
    "                { \"letters\": \"C\", \"class\": \"c_blue clear\" },"
    "                { \"letters\": \"4\", \"class\": \"number\" },"
    "                { \"letters\": \"5\", \"class\": \"number\" },"
    "                { \"letters\": \"6\", \"class\": \"number\" },"
    "                { \"letters\": \"×\", \"class\": \"c_blue multiplication\" },"
    "                { \"letters\": \"÷\", \"class\": \"c_blue division\" },"
    "                { \"letters\": \"1\", \"class\": \"number\" },"
    "                { \"letters\": \"2\", \"class\": \"number\" },"
    "                { \"letters\": \"3\", \"class\": \"number\" },"
    "                { \"letters\": \"+\", \"class\": \"c_blue plus\" },"
    "                { \"letters\": \"-\", \"class\": \"c_blue subtraction\" },"
    "                { \"letters\": \"0\", \"class\": \"number\" },"
    "                { \"letters\": \"00\", \"class\": \"number\" },"
    "                { \"letters\": \".\", \"class\": \"number\" },"
    "                { \"letters\": \"%\", \"class\": \"c_blue percent\" },"
    "                { \"letters\": \"=\", \"class\": \"c_yellow equal\" },"
    "            ]"
    "        </init>"
    "    </head>"
    ""
    "    <body>"
    "        <div id=\"calculator\">"
    ""
    "            <div id=\"c_title\">"
    "                <h2>计算器</h2>"
    "            </div>"
    ""
    "            <div id=\"c_text\">"
    "                <input type=\"text\" id=\"text\" value=\"0\" readonly=\"readonly\" />"
    "            </div>"
    ""
    "            <div id=\"c_value\">"
    "                <archetype name=\"button\">"
    "                    <li class=\"$?.class\">$?.letters</li>"
    "                </archetype>"
    ""
    "                <ul>"
    "                    <iterate on=\"$buttons\">"
    "                        <update on=\"$@\" to=\"append\" with=\"$button\" />"
    "                        <except type=\"NoData\" raw>"
    "                            <p>Bad data!</p>"
    "                        </except>"
    "                    </iterate>"
    "                </ul>"
    "            </div>"
    "        </div>"
    "    </body>"
    ""
    "</hvml>";

static const char *calculator_2 =
    "<!DOCTYPE hvml>"
    "<hvml target=\"html\" lang=\"en\">"
    "    <head>"
    "        <base href=\"$CRTN.base(! 'https://gitlab.fmsoft.cn/hvml/hvml-docs/raw/master/samples/calculator/' )\" />"
    ""
    "<!--"
    "        <update on=\"$T.map\" from=\"assets/{$SYS.locale}.json\" to=\"merge\" />"
    "-->"
    ""
    "        <update on=\"$T.map\" to=\"merge\">"
    "           {"
    "               \"HVML Calculator\": \"HVML 计算器\","
    "               \"Current Time: \": \"当前时间：\""
    "           }"
    "        </update>"
    ""
    "<!--"
    "        <init as=\"buttons\" from=\"assets/buttons.json\" />"
    "-->"
    ""
    "        <init as=\"buttons\" uniquely>"
    "            ["
    "                { \"letters\": \"7\", \"class\": \"number\" },"
    "                { \"letters\": \"8\", \"class\": \"number\" },"
    "                { \"letters\": \"9\", \"class\": \"number\" },"
    "                { \"letters\": \"←\", \"class\": \"c_blue backspace\" },"
    "                { \"letters\": \"C\", \"class\": \"c_blue clear\" },"
    "                { \"letters\": \"4\", \"class\": \"number\" },"
    "                { \"letters\": \"5\", \"class\": \"number\" },"
    "                { \"letters\": \"6\", \"class\": \"number\" },"
    "                { \"letters\": \"×\", \"class\": \"c_blue multiplication\" },"
    "                { \"letters\": \"÷\", \"class\": \"c_blue division\" },"
    "                { \"letters\": \"1\", \"class\": \"number\" },"
    "                { \"letters\": \"2\", \"class\": \"number\" },"
    "                { \"letters\": \"3\", \"class\": \"number\" },"
    "                { \"letters\": \"+\", \"class\": \"c_blue plus\" },"
    "                { \"letters\": \"-\", \"class\": \"c_blue subtraction\" },"
    "                { \"letters\": \"0\", \"class\": \"number\" },"
    "                { \"letters\": \"00\", \"class\": \"number\" },"
    "                { \"letters\": \".\", \"class\": \"number\" },"
    "                { \"letters\": \"%\", \"class\": \"c_blue percent\" },"
    "                { \"letters\": \"=\", \"class\": \"c_yellow equal\" },"
    "            ]"
    "        </init>"
    ""
    "        <title>$T.get('HVML Calculator')</title>"
    ""
    "        <update on=\"$TIMERS\" to=\"displace\">"
    "            ["
    "                { \"id\" : \"clock\", \"interval\" : 1000, \"active\" : \"yes\" },"
    "            ]"
    "        </update>"
    ""
    "        <link rel=\"stylesheet\" type=\"text/css\" href=\"assets/calculator.css\" />"
    "    </head>"
    ""
    "    <body>"
    "        <div id=\"calculator\">"
    ""
    "            <div id=\"c_text\">"
    "                <input type=\"text\" id=\"text\" value=\"0\" readonly=\"readonly\" />"
    "            </div>"
    ""
    "            <div id=\"c_value\">"
    "                <archetype name=\"button\">"
    "                    <li class=\"$?.class\">$?.letters</li>"
    "                </archetype>"
    ""
    "                <ul>"
    "                    <iterate on=\"$buttons\">"
    "                        <update on=\"$@\" to=\"append\" with=\"$button\" />"
    "                        <except type=\"NoData\" raw>"
    "                            <p>Bad data!</p>"
    "                        </except>"
    "                    </iterate>"
    "                </ul>"
    "            </div>"

    "            <div id=\"c_title\">"
    "                <h2 id=\"c_title\">$T.get('HVML Calculator')"
    "                    <small>$T.get('Current Time: ')<span id=\"clock\">$DATETIME.time_prt()</span></small>"
    "                </h2>"
    "                <observe on=\"$TIMERS\" for=\"expired:clock\">"
    "                    <update on=\"#clock\" at=\"textContent\" with=\"$DATETIME.time_prt()\" />"
    "                    <update on=\"$TIMERS\" to=\"overwrite\">"
    "                       { \"id\" : \"clock\", \"active\" : \"no\" }"
    "                    </update>"
    "                    <forget on=\"$TIMERS\" for=\"expired:clock\"/>"
    "                </observe>"
    "            </div>"
    "        </div>"
    "    </body>"
    ""
    "</hvml>";

static const char *calculator_3 =
    "<!DOCTYPE hvml SYSTEM 'v: MATH'>"
    "<hvml target=\"html\" lang=\"en\">"
    "    <head>"
    "        <base href=\"$CRTN.base(! 'https://gitlab.fmsoft.cn/hvml/hvml-docs/raw/master/samples/calculator/' )\" />"
    ""
    "<!--"
    "        <update on=\"$T.map\" from=\"assets/{$SYS.locale}.json\" to=\"merge\" />"
    "-->"
    ""
    "        <update on=\"$T.map\" to=\"merge\">"
    "           {"
    "               \"HVML Calculator\": \"HVML 计算器\","
    "               \"Current Time: \": \"当前时间：\""
    "           }"
    "        </update>"
    ""
    "<!--"
    "        <init as=\"buttons\" from=\"assets/buttons.json\" />"
    "-->"
    ""
    "        <init as=\"buttons\" uniquely>"
    "            ["
    "                { \"letters\": \"7\", \"class\": \"number\" },"
    "                { \"letters\": \"8\", \"class\": \"number\" },"
    "                { \"letters\": \"9\", \"class\": \"number\" },"
    "                { \"letters\": \"←\", \"class\": \"c_blue backspace\" },"
    "                { \"letters\": \"C\", \"class\": \"c_blue clear\" },"
    "                { \"letters\": \"4\", \"class\": \"number\" },"
    "                { \"letters\": \"5\", \"class\": \"number\" },"
    "                { \"letters\": \"6\", \"class\": \"number\" },"
    "                { \"letters\": \"×\", \"class\": \"c_blue multiplication\" },"
    "                { \"letters\": \"÷\", \"class\": \"c_blue division\" },"
    "                { \"letters\": \"1\", \"class\": \"number\" },"
    "                { \"letters\": \"2\", \"class\": \"number\" },"
    "                { \"letters\": \"3\", \"class\": \"number\" },"
    "                { \"letters\": \"+\", \"class\": \"c_blue plus\" },"
    "                { \"letters\": \"-\", \"class\": \"c_blue subtraction\" },"
    "                { \"letters\": \"0\", \"class\": \"number\" },"
    "                { \"letters\": \"00\", \"class\": \"number\" },"
    "                { \"letters\": \".\", \"class\": \"number\" },"
    "                { \"letters\": \"%\", \"class\": \"c_blue percent\" },"
    "                { \"letters\": \"=\", \"class\": \"c_yellow equal\" },"
    "            ]"
    "        </init>"
    ""
    "        <title>$T.get('HVML Calculator')</title>"
    ""
    "        <update on=\"$TIMERS\" to=\"unite\">"
    "            ["
    "                { \"id\" : \"clock\", \"interval\" : 1000, \"active\" : \"yes\" },"
    "                { \"id\" : \"input\", \"interval\" : 1500, \"active\" : \"yes\" },"
    "            ]"
    "        </update>"
    ""
    "        <link rel=\"stylesheet\" type=\"text/css\" href=\"assets/calculator.css\" />"
    "    </head>"
    ""
    "    <body>"
    "        <div id=\"calculator\">"
    ""
    "            <div id=\"c_title\">"
    "                <h2 id=\"c_title\">$T.get('HVML Calculator')"
    "                    <small>$T.get('Current Time: ')<span id=\"clock\">$DATETIME.time_prt()</span></small>"
    "                </h2>"
    "                <observe on=\"$TIMERS\" for=\"expired:clock\">"
    "                    <update on=\"#clock\" at=\"textContent\" with=\"$DATETIME.time_prt()\" />"
    "                </observe>"
    "            </div>"
    ""
    "            <div id=\"c_text\">"
    "                <input type=\"text\" id=\"expression\" value=\"0\" readonly=\"readonly\" />"
    "                <observe on=\"$TIMERS\" for=\"expired:input\">"
    "                    <test on=\"$buttons[$SYS.random($EJSON.count($buttons))].letters\">"
    "                        <match for=\"AS '='\" exclusively>"
    "                            <choose on=\"$MATH.eval($DOC.query('#expression').attr('value'))\">"
    "                                <update on=\"#expression\" at=\"attr.value\" with=\"$?\" />"
    "                                <update on=\"$TIMERS\" to=\"overwrite\">"
    "                                    { \"id\" : \"input\", \"active\" : \"no\" }"
    "                                </update>"
    "                                <catch for=`ANY`>"
    "                                    <update on=\"#expression\" at=\"attr.value\" with=\"ERR\" />"
    "                                </catch>"
    "                            </choose>"
    "                        </match>"
    "                        <match for=\"AS 'C'\" exclusively>"
    "                            <update on=\"#expression\" at=\"attr.value\" with=\"\" />"
    "                        </match>"
    "                        <match for=\"AS '←'\" exclusively>"
    "                            <choose on=\"$DOC.query('#expression').attr.value\">"
    "                                <update on=\"#expression\" at=\"attr.value\" with=\"$STR.substr($?, 0, -1)\" />"
    "                            </choose>"
    "                        </match>"
    ""
    "                        <match>"
    "                            <update on=\"#expression\" at=\"attr.value\" with $= \"$?\" />"
    "                        </match>"
    ""
    "                        <match for=\"ANY\" exclusively>"
    "                            <update on=\"$TIMERS\" to=\"overwrite\">"
    "                                { \"id\" : \"input\", \"active\" : \"no\" }"
    "                            </update>"
    "                        </match>"
    "                    </test>"
    "                </observe>"
    "            </div>"
    ""
    "            <div id=\"c_value\">"
    "                <archetype name=\"button\">"
    "                    <li class=\"$?.class\">$?.letters</li>"
    "                </archetype>"
    ""
    "                <ul>"
    "                    <iterate on=\"$buttons\">"
    "                        <update on=\"$@\" to=\"append\" with=\"$button\" />"
    "                        <except type=\"NoData\" raw>"
    "                            <p>Bad data!</p>"
    "                        </except>"
    "                    </iterate>"
    "                </ul>"
    "            </div>"
    "        </div>"
    "    </body>"
    ""
    "</hvml>";

static const char *calculator_4 =
    "<!DOCTYPE hvml SYSTEM 'v: MATH'>"
    "<hvml target=\"html\" lang=\"en\">"
    "    <head>"
    "        <base href=\"$CRTN.base(! 'https://gitlab.fmsoft.cn/hvml/hvml-docs/raw/master/samples/calculator/' )\" />"
    ""
    "<!--"
    "        <update on=\"$T.map\" from=\"assets/{$SYS.locale}.json\" to=\"merge\" />"
    "-->"
    ""
    "        <update on=\"$T.map\" to=\"merge\">"
    "           {"
    "               \"HVML Calculator\": \"HVML 计算器\","
    "               \"Current Time: \": \"当前时间：\""
    "           }"
    "        </update>"
    ""
    "<!--"
    "        <init as=\"buttons\" from=\"assets/buttons.json\" />"
    "-->"
    ""
    "        <init as=\"buttons\" uniquely>"
    "            ["
    "                { \"letters\": \"7\", \"class\": \"number\" },"
    "                { \"letters\": \"8\", \"class\": \"number\" },"
    "                { \"letters\": \"9\", \"class\": \"number\" },"
    "                { \"letters\": \"←\", \"class\": \"c_blue backspace\" },"
    "                { \"letters\": \"C\", \"class\": \"c_blue clear\" },"
    "                { \"letters\": \"4\", \"class\": \"number\" },"
    "                { \"letters\": \"5\", \"class\": \"number\" },"
    "                { \"letters\": \"6\", \"class\": \"number\" },"
    "                { \"letters\": \"×\", \"class\": \"c_blue multiplication\" },"
    "                { \"letters\": \"÷\", \"class\": \"c_blue division\" },"
    "                { \"letters\": \"1\", \"class\": \"number\" },"
    "                { \"letters\": \"2\", \"class\": \"number\" },"
    "                { \"letters\": \"3\", \"class\": \"number\" },"
    "                { \"letters\": \"+\", \"class\": \"c_blue plus\" },"
    "                { \"letters\": \"-\", \"class\": \"c_blue subtraction\" },"
    "                { \"letters\": \"0\", \"class\": \"number\" },"
    "                { \"letters\": \"00\", \"class\": \"number\" },"
    "                { \"letters\": \".\", \"class\": \"number\" },"
    "                { \"letters\": \"%\", \"class\": \"c_blue percent\" },"
    "                { \"letters\": \"=\", \"class\": \"c_yellow equal\" },"
    "            ]"
    "        </init>"
    ""
    "<!--"
    "        <init as=\"expressions\" from=\"assets/expressions.json\" />"
    "-->"
    ""
    "        <init as=\"expressions\">"
    "           ["
    "               \"7*3=\","
    "           ]"
    "<!--"
    "               \"800+100=\","
    "               \"100/10=\","
    "               \"50C80/10=\","
    "               \"5←80/10=\","
    "               \"←←80/10=\""
    "-->"
    "        </init>"
    ""
    "        <title>$T.get('HVML Calculator')</title>"
    ""
    "        <update on=\"$TIMERS\" to=\"unite\">"
    "            ["
    "                { \"id\" : \"clock\", \"interval\" : 1000, \"active\" : \"yes\" },"
    "                { \"id\" : \"input\", \"interval\" : 1500, \"active\" : \"yes\" },"
    "            ]"
    "        </update>"
    ""
    "        <link rel=\"stylesheet\" type=\"text/css\" href=\"assets/calculator.css\" />"
    "    </head>"
    ""
    "    <body>"
    "        <init as=\"exp_chars\" with=\"[]\" />"
    ""
    "        <iterate on=\"$expressions\" by=\"RANGE: FROM 0\" >"
    "            <update on=\"$exp_chars\" to=\"append\" with=\"[]\" />"
    ""
    "            <iterate on=\"$?\" by=\"CHAR: FROM 0\" >"
    "                <update on=\"$exp_chars\" at=\"$2%\" to=\"append\" with=\"$?\" />"
    "            </iterate>"
    "        </iterate>"
    ""
    "        <init as=\"info\">"
    "            {"
    "                \"chars\" : $exp_chars[$SYS.random($EJSON.count($exp_chars))],"
    "                \"index\" : 0,"
    "            }"
    "        </init>"
    ""
    "        <div id=\"calculator\">"
    ""
    "            <div id=\"c_title\">"
    "                <h2 id=\"c_title\">$T.get('HVML Calculator')"
    "                    <small>$T.get('Current Time: ')<span id=\"clock\">$DATETIME.time_prt()</span></small>"
    "                </h2>"
    "                <observe on=\"$TIMERS\" for=\"expired:clock\">"
    "                    <update on=\"#clock\" at=\"textContent\" to=\"displace\" with=\"$DATETIME.time_prt()\" />"
    "                </observe>"
    "            </div>"
    ""
    "            <div id=\"c_text\">"
    "                <input type=\"text\" id=\"expression\" value=\"\" readonly=\"readonly\" />"
    "                <observe on=\"$TIMERS\" for=\"expired:input\">"
    "                    <test on=\"$info.chars[$info.index]\">"
    "                        <update on=\"$info\" at=\".index\" to=\"displace\" with=\"$MATH.add($info.index, 1)\" />"
    ""
    "                        <match for=\"AS '='\" exclusively>"
    "                            <choose on=\"$MATH.eval($DOC.query('#expression').attr('value'))\">"
    "                                <update on=\"#expression\" at=\"attr.value\" with=\"$?\" />"
    "                                <update on=\"$TIMERS\" to=\"overwrite\">"
    "                                    { \"id\" : \"input\", \"active\" : \"no\" }"
    "                                </update>"
    "                                <update on=\"$TIMERS\" to=\"overwrite\">"
    "                                    { \"id\" : \"clock\", \"active\" : \"no\" }"
    "                                </update>"
    "                                <catch for=`ANY`>"
    "                                    <update on=\"#expression\" at=\"attr.value\" with=\"ERR\" />"
    "                                </catch>"
    "                                <update on=\"$TIMERS\" to=\"overwrite\">"
    "                                    { \"id\" : \"input\", \"active\" : \"no\" }"
    "                                </update>"
    "                                <update on=\"$TIMERS\" to=\"overwrite\">"
    "                                    { \"id\" : \"clock\", \"active\" : \"no\" }"
    "                                </update>"
    "                                <forget on=\"$TIMERS\" for=\"expired:clock\"/>"
    "                                <forget on=\"$TIMERS\" for=\"expired:input\"/>"
    "                            </choose>"
    "                        </match>"
    ""
    "                        <match for=\"AS 'C'\" exclusively>"
    "                            <update on=\"#expression\" at=\"attr.value\" with=\"\" />"
    "                        </match>"
    ""
    "                        <match for=\"AS '←'\" exclusively>"
    "                            <choose on=\"$DOC.query('#expression').attr.value\">"
    "                                <update on=\"#expression\" at=\"attr.value\" with=\"$STR.substr($?, 0, -1)\" />"
    "                            </choose>"
    "                        </match>"
    ""
    "                        <match>"
    "                            <update on=\"#expression\" at=\"attr.value\" with $= \"$?\" />"
    "                        </match>"
    "                    </test>"
    "                </observe>"
    "            </div>"
    ""
    "            <div id=\"c_value\">"
    "                <archetype name=\"button\">"
    "                    <li class=\"$?.class\">$?.letters</li>"
    "                </archetype>"
    ""
    "                <ul>"
    "                    <iterate on=\"$buttons\">"
    "                        <update on=\"$@\" to=\"append\" with=\"$button\" />"
    "                        <except type=\"NoData\" raw>"
    "                            <p>Bad data!</p>"
    "                        </except>"
    "                    </iterate>"
    "                </ul>"
    "            </div>"
    "        </div>"
    "    </body>"
    ""
    "</hvml>";

static const char *buggy1 =
    "<!DOCTYPE hvml SYSTEM 'v: MATH'>"
    "<hvml target=\"html\" lang=\"en\">"
    "    <head>"
    "        <base href=\"$CRTN.base(! 'https://gitlab.fmsoft.cn/hvml/hvml-docs/raw/master/samples/calculator/' )\" />"
    ""
    "        <update on=\"$T.map\" to=\"merge\">"
    "           {"
    "               \"HVML Calculator\": \"HVML 计算器\","
    "               \"Current Time: \": \"当前时间：\""
    "           }"
    "        </update>"
    ""
    "        <init as=\"buttons\" uniquely>"
    "            ["
    "                { \"letters\": \"7\", \"class\": \"number\" },"
    "                { \"letters\": \"8\", \"class\": \"number\" },"
    "                { \"letters\": \"9\", \"class\": \"number\" },"
    "                { \"letters\": \"←\", \"class\": \"c_blue backspace\" },"
    "                { \"letters\": \"C\", \"class\": \"c_blue clear\" },"
    "                { \"letters\": \"4\", \"class\": \"number\" },"
    "                { \"letters\": \"5\", \"class\": \"number\" },"
    "                { \"letters\": \"6\", \"class\": \"number\" },"
    "                { \"letters\": \"×\", \"class\": \"c_blue multiplication\" },"
    "                { \"letters\": \"÷\", \"class\": \"c_blue division\" },"
    "                { \"letters\": \"1\", \"class\": \"number\" },"
    "                { \"letters\": \"2\", \"class\": \"number\" },"
    "                { \"letters\": \"3\", \"class\": \"number\" },"
    "                { \"letters\": \"+\", \"class\": \"c_blue plus\" },"
    "                { \"letters\": \"-\", \"class\": \"c_blue subtraction\" },"
    "                { \"letters\": \"0\", \"class\": \"number\" },"
    "                { \"letters\": \"00\", \"class\": \"number\" },"
    "                { \"letters\": \".\", \"class\": \"number\" },"
    "                { \"letters\": \"%\", \"class\": \"c_blue percent\" },"
    "                { \"letters\": \"=\", \"class\": \"c_yellow equal\" },"
    "            ]"
    "        </init>"
    ""
    "        <init as=\"expressions\">"
    "           ["
    "               \"7*6=\","
    "           ]"
    "        </init>"
    ""
    "        <title>$T.get('HVML Calculator')</title>"
    ""
    "        <update on=\"$TIMERS\" to=\"unite\">"
    "            ["
    "                { \"id\" : \"clock\", \"interval\" : 1000, \"active\" : \"yes\" },"
    "                { \"id\" : \"input\", \"interval\" : 1500, \"active\" : \"yes\" },"
    "            ]"
    "        </update>"
    ""
    "        <link rel=\"stylesheet\" type=\"text/css\" href=\"assets/calculator.css\" />"
    "    </head>"
    ""
    "    <body>"
    "        <init as=\"exp_chars\" with=\"[]\" />"
    ""
    "        <iterate on=\"$expressions\" by=\"RANGE: FROM 0\" >"
    "            <update on=\"$exp_chars\" to=\"append\" with=\"[]\" />"
    ""
    "            <iterate on=\"$?\" by=\"CHAR: FROM 0\" >"
    "                <update on=\"$exp_chars\" at=\"$1%\" to=\"append\" with=\"$?\" />"
    "            </iterate>"
    "        </iterate>"
    ""
    "        <init as=\"info\">"
    "            {"
    "                \"chars\" : $exp_chars[$SYS.random($EJSON.count($exp_chars))],"
    "                \"index\" : 0,"
    "            }"
    "        </init>"
    ""
    "        <div id=\"calculator\">"
    ""
    "            <div id=\"c_title\">"
    "                <h2 id=\"c_title\">$T.get('HVML Calculator')"
    "                    <small>$T.get('Current Time: ')<span id=\"clock\">$DATETIME.time_prt()</span></small>"
    "                </h2>"
    "                <iterate on=\"[1,2,3]\">"
    "                    <update on=\"#clock\" at=\"textContent\" to=\"displace\" with=\"iterator:$?\" />"
    "                </iterate>"
    "            </div>"
    ""
    "            <div id=\"c_text\">"
    "                <input type=\"text\" id=\"expression\" value=\"\" readonly=\"readonly\" />"
    "                <iterate on=\"$info.chars\" >"
    "                    <test on=\"$info.chars[$info.index]\">"
    "                        <update on=\"$info\" at=\".index\" to=\"displace\" with=\"$MATH.add($info.index, 1)\" />"
    ""
    "                        <match for=\"AS '='\" exclusively>"
    "                            <choose on=\"$MATH.eval($DOC.query('#expression').attr('value'))\">"
    "                                <update on=\"#expression\" at=\"attr.value\" with=\"$?\" />"
    "                                <update on=\"$TIMERS\" to=\"overwrite\">"
    "                                    { \"id\" : \"input\", \"active\" : \"no\" }"
    "                                </update>"
    "                                <update on=\"$TIMERS\" to=\"overwrite\">"
    "                                    { \"id\" : \"clock\", \"active\" : \"no\" }"
    "                                </update>"
    "                                <catch for=`ANY`>"
    "                                    <update on=\"#expression\" at=\"attr.value\" with=\"ERR\" />"
    "                                </catch>"
    "                            </choose>"
    "                        </match>"
    ""
    "                        <match for=\"AS 'C'\" exclusively>"
    "                            <update on=\"#expression\" at=\"attr.value\" with=\"\" />"
    "                        </match>"
    ""
    "                        <match for=\"AS '←'\" exclusively>"
    "                            <choose on=\"$DOC.query('#expression').attr.value\">"
    "                                <update on=\"#expression\" at=\"attr.value\" with=\"$STR.substr($?, 0, -1)\" />"
    "                            </choose>"
    "                        </match>"
    ""
    "                        <match>"
    "                            <update on=\"#expression\" at=\"attr.value\" with $= \"$?\" />"
    "                        </match>"
    "                    </test>"
    "                </iterate>"
    "            </div>"
    ""
    "            <div id=\"c_value\">"
    "                <archetype name=\"button\">"
    "                    <li class=\"$?.class\">$?.letters</li>"
    "                </archetype>"
    ""
    "                <ul>"
    "                    <iterate on=\"$buttons\">"
    "                        <update on=\"$@\" to=\"append\" with=\"$button\" />"
    "                        <except type=\"NoData\" raw>"
    "                            <p>Bad data!</p>"
    "                        </except>"
    "                    </iterate>"
    "                </ul>"
    "            </div>"
    "        </div>"
    "    </body>"
    ""
    "</hvml>";

static const char *buggy2 =
    "<hvml><head><title>hello</title></head><body><span id=\"clock\">xyz</span><xinput xid=\"xexp\"></xinput><update on=\"#clock\" at=\"textContent\" to=\"displace\" with=\"abc\"/></body></hvml>";

static const char *buggy3 =
    "<hvml><body><span id=\"clock\">xyz</span><update on=\"#clock\" at=\"textContent\" to=\"displace\" with=\"$DATETIME.time_prt()\" /></body></hvml>";

static const char *buggy4 =
    "<hvml target=\"html\" lang=\"en\">"
    "    <head>"
    "    </head>"
    "    <body>"
    "        <span id=\"clock\">def</span>"
    "        <div>"
    "            <xinput xtype=\"xt\" xype=\"abd\" />"
    "        </div>"
    "        <update on=\"#clock\" at=\"textContent\" to=\"displace\" with=\"xyz\" />"
    "    </body>"
    "</hvml>";

static const char *sample1 =
    "<!DOCTYPE hvml>"
    "<hvml target=\"html\">"
    "    <head>"
    "    </head>"
    ""
    "    <body>"
    "        <init as=\"global\">"
    "            { \"locale\" : \"zh_CN\" }"
    "        </init>"
    ""
    "        <archetype name=\"footer_cn\">"
    "            <p><a href=\"http://www.baidu.com\">百度</a></p>"
    "        </archetype>"
    ""
    "        <archetype name=\"footer_tw\">"
    "            <p><a href=\"http://www.bing.com\">Bing</a></p>"
    "        </archetype>"
    ""
    "        <archetype name=\"footer_def\">"
    "            <p><a href=\"http://www.google.com\">Google</a></p>"
    "        </archetype>"
    ""
    "        <footer id=\"the-footer\">"
    "            <test on=\"$global.locale\" in='#the-footer'>"
    "                <match for=\"AS 'zh_CN'\" exclusively>"
    "                    <update on=\"$@\" to=\"append\" with=\"$footer_cn\" />"
    "                </match>"
    "                <match for=\"AS 'zh_TW'\" exclusively>"
    "                    <update on=\"$@\" to=\"append\" with=\"$footer_tw\" />"
    "                </match>"
    "                <match for=\"ANY\" to=\"displace\" with=\"$footer_def\">"
    "                    <update on=\"$@\" to=\"append\" with=\"$footer_def\" />"
    "                </match>"
    "                <except type=\"NoData\" raw>"
    "                    <p>You forget to define the $global variable!</p>"
    "                </except>"
    "                <except type=\"NoSuchKey\">"
    "                    <p>Bad global data!</p>"
    "                </except>"
    "                <except type=\"IdentifierError\">"
    "                    <p>Bad archetype data!</p>"
    "                </except>"
    "            </test>"
    "        </footer>"
    "    </body>"
    "</hvml>";

static const char *sample2 =
    "<!DOCTYPE hvml>"
    "<hvml target=\"html\" lang=\"en\">"
    "    <head>"
    "        <base href=\"$CRTN.base(! 'https://gitlab.fmsoft.cn/hvml/hvml-docs/raw/master/samples/calculator/' )\" />"
    ""
    "        <update on=\"$T.map\" from=\"assets/{$SYS.locale}.json\" to=\"merge\" />"
    ""
    "        <init as=\"buttons\" from=\"assets/buttons.json\" />"
    ""
    "        <title>$T.get('HVML Calculator')</title>"
    ""
    "        <update on=\"$TIMERS\" to=\"displace\">"
    "            ["
    "                { \"id\" : \"clock\", \"interval\" : 1000, \"active\" : \"yes\" },"
    "            ]"
    "        </update>"
    ""
    "        <link rel=\"stylesheet\" type=\"text/css\" href=\"assets/calculator.css\" />"
    "    </head>"
    ""
    "    <body>"
    "    </body>"
    ""
    "</hvml>";

static const char *fibonacci_1 =
    "<!DOCTYPE hvml SYSTEM 'v: MATH'>"
    "<hvml target=\"html\" lang=\"en\">"
    "    <head>"
    "        <title>Fibonacci Numbers</title>"
    "    </head>"
    ""
    "    <body>"
    "        <header>"
    "            <h1>Fibonacci Numbers less than 2000</h1>"
    "            <!--p hvml:raw>Using named array variable ($fibonacci), $MATH, and $EJSON</p-->"
    "        </header>"
    ""
    "        <init as=\"fibonacci\">"
    "            [0, 1, ]"
    "        </init>"
    ""
    "        <iterate on 1 by=\"ADD: LT 2000 BY $fibonacci[$MATH.sub($EJSON.count($fibonacci), 2)]\">"
    "            <update on=\"$fibonacci\" to=\"append\" with=\"$?\" />"
    "        </iterate>"
    ""
    "        <section>"
    "            <ol>"
    "                <archetype name=\"fibo-item\">"
    "                    <li>$?</li>"
    "                </archetype>"
    "                <iterate on=\"$fibonacci\">"
    "                   <update on=\"$@\" to=\"append\" with=\"$fibo-item\" />"
    "                </iterate>"
    "            </ol>"
    "        </section>"
    ""
    "        <footer>"
    "            <p>Totally $EJSON.count($fibonacci) numbers.</p>"
    "        </footer>"
    "    </body>"
    ""
    "</hvml>";

static const char *fibonacci_2 =
    "<!DOCTYPE hvml>"
    "<hvml target=\"html\" lang=\"en\">"
    "    <head>"
    "        <title>Fibonacci Numbers</title>"
    "    </head>"
    ""
    "    <body>"
    "        <header>"
    "            <h1>Fibonacci Numbers less than 2000</h1>"
    "            <p hvml:raw>Using local array variable ($!) and negative index</p>"
    "        </header>"
    ""
    "        <init as='fibonacci' temporarily>"
    "            [0, 1, ]"
    "        </init>"
    ""
    "        <iterate on 1 by=\"ADD: LT 2000 BY $!.fibonacci[-2]\">"
    "            <update on=\"$1!.fibonacci\" to=\"append\" with=\"$?\" />"
    "        </iterate>"
    ""
    "        <section>"
    "            <ol>"
    "                <iterate on=\"$2!.fibonacci\">"
    "                    <li>$?</li>"
    "                </iterate>"
    "            </ol>"
    "        </section>"
    ""
    "        <footer>"
    "            <p>Totally $EJSON.count($1!.fibonacci) numbers.</p>"
    "        </footer>"
    "    </body>"
    ""
    "</hvml>";

static const char *fibonacci_3 =
    "<!DOCTYPE hvml>"
    "<hvml target=\"html\" lang=\"en\">"
    "    <head>"
    "        <title>Fibonacci Numbers</title>"
    "    </head>"
    ""
    "    <body>"
    "        <header>"
    "            <h1>Fibonacci Numbers less than 2000</h1>"
    "            <p hvml:raw>Using non-array local variables</p>"
    "        </header>"
    ""
    "        <init as=\"last_one\" with=0 temporarily />"
    "        <init as=\"last_two\" with=1 temporarily />"
    "        <init as=\"count\" with=2 temporarily />"
    ""
    "        <section>"
    "            <dl>"
    "                <iterate on 1 by=\"ADD: LT 2000 BY $2!.last_one\">"
    "                    <update on=\"$3!\" at=\".last_one\" to=\"displace\" with=\"$3!.last_two\" />"
    "                    <update on=\"$3!\" at=\".last_two\" to=\"displace\" with=\"$?\" />"
    "                    <update on=\"$3!\" at=\".count\" to=\"displace\" with += 1 />"
    "                    <dt>$%</dt>"
    "                    <dd>$?</dd>"
    "                </iterate>"
    "            </dl>"
    "        </section>"
    ""
    "        <footer>"
    "            <p>Totally $1!.count numbers.</p>"
    "        </footer>"
    "    </body>"
    ""
    "</hvml>";

TEST(interpreter, basic)
{
    (void)calculator_1;
    (void)calculator_2;
    (void)calculator_3;
    (void)calculator_4;
    (void)buggy1;
    (void)buggy2;
    (void)buggy3;
    (void)buggy4;
    (void)sample1;
    (void)sample2;
    (void)fibonacci_1;
    (void)fibonacci_2;
    (void)fibonacci_3;

    const char *hvmls[] = {
        // "<hvml>"
        // "  <head>"
        // "  </head>"
        // "  <body>"
        // "    <div>"
        // "      foo"
        // "      <archetype name=\"foo\"><hoo><bar></bar><foobar>ddddddddddddddddddddddddddd</foobar></hoo></archetype>"
        // "      bar"
        // "      <update on=\"$@\" to=\"append\" with=\"$foo\" />"
        // "    </div>"
        // "    world"
        // "  </body>"
        // "</hvml>",
        // "<hvml><head x=\"y\"><xinit a=\"b\">world<!--yes-->solid</xinit></head><body><timeout1/><timeout3/></body></hvml>",
        // "<hvml><head x=\"y\">hello<xinit a=\"b\">w<timeout3/>orld<!--yes-->solid</xinit></head><body><timeout1/></body></hvml>",
        // "<hvml><body><timeout1/><timeout9/><timeout2/></body></hvml>",
        // "<hvml><body><xtest a='b'>hello<!--yes--></xtest></body></hvml>",
        // "<hvml><body><archetype name=\"$?.button\"><li class=\"class\">letters</li></archetype></body></hvml>",
        // "<hvml><body><archetype name=\"button\"><li class=\"class\">letters</li></archetype></body></hvml>",
        // "<hvml><body><a><b><c></c></b></a></body></hvml>",
        // calculator_1,
        // calculator_2,
        // // calculator_3,
        calculator_4,
        // sample1,
        // // sample2,
//         fibonacci_1,
//         fibonacci_2,
//         fibonacci_3,
        // buggy1,
        // buggy2,
        // buggy3,
        // buggy4,
    };

    PurCInstance purc("cn.fmsoft.hybridos.test", "interpreter", false);

    ASSERT_TRUE(purc);

    // get statitics information
    const struct purc_variant_stat * stat = purc_variant_usage_stat ();
    ASSERT_NE(stat, nullptr);

    for (size_t i=0; i<PCA_TABLESIZE(hvmls); ++i) {
        const char *hvml = hvmls[i];
        purc_vdom_t vdom = purc_load_hvml_from_string(hvml);
        purc_schedule_vdom_null(vdom);
        ASSERT_NE(vdom, nullptr);
    }

    purc_run(NULL);
}

