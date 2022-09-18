#include "parse.h"

using namespace mapish;

#define NEXT_AND(Val) return next(), (Val);
#define NEXT_AND_ERROR(Err) return error(Err), next(), std::nullopt
#define NEXT_OR(Err) if (!next() && (error(Err),true)) return std::nullopt
#define RETURN_ERROR(Err) return error(Err), std::nullopt
#define GET_OR_RETURN(Var, Expr) \
  if (auto _x = (Expr); _x.has_value()) Var = _x.value(); \
  else return std::nullopt
#define GET_OR_ERROR(Var, Expr, Err) \
  if (auto _x = (Expr); _x.has_value()) Var = _x.value(); \
  else return error(Err), std::nullopt
#define EXPECT(Tk) \
  if (current().kind != tk_##Tk) \
    return error("expected " #Tk), std::nullopt; \
  else next()
#define EXPECT_KEYWORD(Kw) \
  if (current().kind != tk_word || current().value.kw != kw_##Kw) \
    return error("expected '" #Kw "'"), std::nullopt; \
  else next()
#define IS_KEYWORD(Kw) \
  (current().kind == tk_word && current().value.kw == kw_##Kw)

bool Parser::parseSeparator() {
  bool had_sep = false;
  while (current().kind == tk_eof || current().kind == tk_semi) {
    had_sep = true;
    next();
  }
  return had_sep;
}

std::optional<ListKind> Parser::parseListKind() {
  if (current().text != "list") {
    return std::nullopt;
  }

  NEXT_OR("expected list kind");

  if (IS_KEYWORD(in)) {
    NEXT_AND(ListKind::Inputs);
  } else if (IS_KEYWORD(out)) {
    NEXT_AND(ListKind::Outputs);
  }

  RETURN_ERROR("unknown list kind");
}

std::optional<uint8_t> Parser::parseNoteName() {
  auto text = current().text;
  size_t i;
  uint8_t number;

  // note 0 = C-1.
  // note 12 = C0.
  // note 21 = A0.
  switch (text[0]) {
  case 'A':
    number = 21;
    break;
  case 'B':
    number = 23;
    break;
  case 'C':
    number = 12;
    break;
  case 'D':
    number = 14;
    break;
  case 'E':
    number = 16;
    break;
  case 'F':
    number = 17;
    break;
  case 'G':
    number = 19;
    break;
  default:
    NEXT_AND_ERROR("note names start with A - G");
  }

  if (text.length() == 1) {
    NEXT_AND_ERROR("missing octave number");
  }

  // Flat or sharp?
  if (text[1] == '#') {
    // TODO: Also accept '♯' - this is e2 99 af.
    i = 2;
    ++number;
  } else if (text[1] == 'b') {
    // TODO: Also accept '♭' - this is e2 99 ad.
    i = 2;
    --number;
  } else {
    i = 1;
  }

  if (text.length() == i) {
    NEXT_AND_ERROR("missing octave number");
  }

  if (text[i] == '-') {
    if (text.length() == i + 1) {
      NEXT_AND_ERROR("'-' is not a valid octave, but '-1' is");
    }
    if (text[i] != '1') {
      NEXT_AND_ERROR("'-1' is the only negative octave");
    }
    i += 2;

    if (number < 12) {
      NEXT_AND_ERROR("C-1 is the lowest note");
    }
    number -= 12;
  } else if (text[i] >= '0' && text[i] <= '9') {
    number += 12 * (text[i] - '0');
    ++i;
    if (number > 127) {
      NEXT_AND_ERROR("G9 is the highest note");
    }
  }

  if (text.length() != i) {
    NEXT_AND_ERROR("extraneous text after note name");
  }

  NEXT_AND(number);
}

std::optional<a8::midi::Message> Parser::parseMidiMessage() {
  using a8::midi::Message;

  // TODO read channel.
  uint8_t channel = 0;

  if (IS_KEYWORD(note)) {
    uint8_t note;
    NEXT_OR("expected note name");
    GET_OR_RETURN(note, parseNoteName());
    NEXT_OR("expected 'on' or 'off'");
    if (IS_KEYWORD(on)) {
      // TODO read value
      NEXT_AND(Message::NoteOn(channel, note, 0x7f));
    } else if (IS_KEYWORD(off)) {
      // TODO read value
      NEXT_AND(Message::NoteOff(channel, note, 0));
    } else {
      RETURN_ERROR("expected 'on' or 'off'");
    }
  }

  else if (IS_KEYWORD(polyphonic)) {
    NEXT_OR("expected 'aftertouch'");
    EXPECT_KEYWORD(aftertouch);
    uint8_t note;
    GET_OR_RETURN(note, parseNoteName());
    // TODO read value
    return Message::PolyphonicAftertouch(1, note, 0x7f);
  }

  else if (IS_KEYWORD(channel)) {
    NEXT_OR("expected 'aftertouch'");
    EXPECT_KEYWORD(aftertouch);
    // TODO read value
    return Message::ChannelAftertouch(1, 0x7f);
  }

  else if (IS_KEYWORD(program)) {
    // TODO read program number
    // TODO program bank message
    NEXT_AND(Message::ProgramChange(channel, 0));
  }

  else if (IS_KEYWORD(pitchbend)) {
    // TODO read amount
    //NEXT_OR("expected pitch bend amount");
    NEXT_AND(Message::PitchBend(channel, 0));
  }

  else if (IS_KEYWORD(quarterframe)) {
    // TODO What are the parameters here?
    NEXT_AND(Message::QuarterFrame(0, 0));
  }

  else if (IS_KEYWORD(song)) {
    NEXT_OR("expected 'position' or 'select'");
    if (IS_KEYWORD(position)) {
      // TODO read value
      NEXT_AND(Message::SongPosition(0));
    } else if (IS_KEYWORD(select)) {
      // TODO read value
      NEXT_AND(Message::SongSelect(0));
    } else {
      RETURN_ERROR("expected 'position' or 'select'");
    }
  }

  else if (IS_KEYWORD(tunerequest)) {
    NEXT_AND(Message::TuneRequest());
  }

  else if (IS_KEYWORD(timingclock)) {
    NEXT_AND(Message::TimingClock());
  }

  else if (IS_KEYWORD(start)) {
    NEXT_AND(Message::Start());
  }

  else if (IS_KEYWORD(continue)) {
    NEXT_AND(Message::Continue());
  }

  else if (IS_KEYWORD(stop)) {
    NEXT_AND(Message::Stop());
  }

  else if (IS_KEYWORD(activesensing)) {
    NEXT_AND(Message::ActiveSensing());
  }

  else if (IS_KEYWORD(reset)) {
    NEXT_AND(Message::Reset());
  }

  else if (IS_KEYWORD(sysex)) {
    error("SysEx not supported");
    // TODO [bytes...]
    NEXT_AND(Message::SysEx({}));
  }

  else {
    RETURN_ERROR("unknown message name");
  }
}

void Parser::error(std::string_view what) {
  A8_EPRINT("{}:{}: error: {} (found ", current().line, current().column,
            what);
  switch (token_print_kind(current().kind)) {
  case print_text:
    A8_EPRINT("{} '{}').\n", token_name(current().kind), current().text);
    break;
  case print_hex:
    A8_EPRINT("{} 0x{:X}).\n", token_name(current().kind), current().text[0]);
    break;
  case print_none:
    A8_EPRINT("'{}').\n", token_name(current().kind));
    break;
  }
}
