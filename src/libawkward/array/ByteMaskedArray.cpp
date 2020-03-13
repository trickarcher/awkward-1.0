// BSD 3-Clause License; see https://github.com/jpivarski/awkward-1.0/blob/master/LICENSE

#include <sstream>
#include <type_traits>

#include "awkward/cpu-kernels/identities.h"
#include "awkward/cpu-kernels/getitem.h"
#include "awkward/cpu-kernels/operations.h"
#include "awkward/type/OptionType.h"
#include "awkward/type/ArrayType.h"
#include "awkward/type/UnknownType.h"
#include "awkward/Slice.h"
#include "awkward/array/None.h"
#include "awkward/array/EmptyArray.h"
#include "awkward/array/UnionArray.h"
#include "awkward/array/NumpyArray.h"
#include "awkward/array/IndexedArray.h"

#include "awkward/array/ByteMaskedArray.h"

namespace awkward {
  ByteMaskedArray::ByteMaskedArray(const std::shared_ptr<Identities>& identities, const util::Parameters& parameters, const Index8& mask, const std::shared_ptr<Content>& content, bool validwhen)
      : Content(identities, parameters)
      , mask_(mask)
      , content_(content)
      , validwhen_(validwhen != 0) {
    if (content.get()->length() < mask.length()) {
      throw std::invalid_argument("ByteMaskedArray content must not be shorter than its mask");
    }
  }

  const Index8 ByteMaskedArray::mask() const {
    return mask_;
  }

  const std::shared_ptr<Content> ByteMaskedArray::content() const {
    return content_;
  }

  bool ByteMaskedArray::validwhen() const {
    return validwhen_;
  }

  const std::shared_ptr<Content> ByteMaskedArray::project() const {
    int64_t numnull;
    struct Error err1 = awkward_bytemaskedarray_numnull(
      &numnull,
      mask_.ptr().get(),
      mask_.offset(),
      length(),
      validwhen_);
    util::handle_error(err1, classname(), identities_.get());

    Index64 nextcarry(length() - numnull);
    struct Error err2 = awkward_bytemaskedarray_getitem_nextcarry_64(
      nextcarry.ptr().get(),
      mask_.ptr().get(),
      mask_.offset(),
      length(),
      validwhen_);
    util::handle_error(err2, classname(), identities_.get());

    return content_.get()->carry(nextcarry);
  }

  const std::shared_ptr<Content> ByteMaskedArray::project(const Index8& mask) const {
    if (length() != mask.length()) {
      throw std::invalid_argument(std::string("mask length (") + std::to_string(mask.length()) + std::string(") is not equal to ") + classname() + std::string(" length (") + std::to_string(length()) + std::string(")"));
    }

    Index8 nextmask(length());
    struct Error err = awkward_bytemaskedarray_overlay_mask8(
      nextmask.ptr().get(),
      mask.ptr().get(),
      mask.offset(),
      mask_.ptr().get(),
      mask_.offset(),
      length(),
      validwhen_);
    util::handle_error(err, classname(), identities_.get());

    ByteMaskedArray next(identities_, parameters_, nextmask, content_, false);  // validwhen=false
    return next.project();
  }

  const Index8 ByteMaskedArray::bytemask() const {
    Index8 out(length());
    struct Error err = awkward_bytemaskedarray_mask8(
      out.ptr().get(),
      mask_.ptr().get(),
      mask_.offset(),
      mask_.length(),
      validwhen_);
    util::handle_error(err, classname(), identities_.get());
    return out;
  }

  const std::shared_ptr<Content> ByteMaskedArray::simplify() const {
    throw std::runtime_error("FIXME: ByteMaskedArray::simplify");
  }

  const std::shared_ptr<Content> ByteMaskedArray::toIndexedOptionArray64() const {
    Index64 index(length());
    struct Error err = awkward_bytemaskedarray_toindexedarray_64(
      index.ptr().get(),
      mask_.ptr().get(),
      mask_.offset(),
      mask_.length(),
      validwhen_);
    util::handle_error(err, classname(), identities_.get());
    return std::make_shared<IndexedOptionArray64>(identities_, parameters_, index, content_);
  }

  const std::string ByteMaskedArray::classname() const {
    return "ByteMaskedArray";
  }

  void ByteMaskedArray::setidentities(const std::shared_ptr<Identities>& identities) {
    throw std::runtime_error("FIXME: ByteMaskedArray::setidentities(identities)");
  }

  void ByteMaskedArray::setidentities() {
    throw std::runtime_error("FIXME: ByteMaskedArray::setidentities");
  }

  const std::shared_ptr<Type> ByteMaskedArray::type(const std::map<std::string, std::string>& typestrs) const {
    return std::make_shared<OptionType>(parameters_, util::gettypestr(parameters_, typestrs), content_.get()->type(typestrs));
  }

  const std::string ByteMaskedArray::tostring_part(const std::string& indent, const std::string& pre, const std::string& post) const {
    std::stringstream out;
    out << indent << pre << "<" << classname() << " validwhen=\"" << (validwhen_ ? "true" : "false") << "\">\n";
    if (identities_.get() != nullptr) {
      out << identities_.get()->tostring_part(indent + std::string("    "), "", "\n");
    }
    if (!parameters_.empty()) {
      out << parameters_tostring(indent + std::string("    "), "", "\n");
    }
    out << mask_.tostring_part(indent + std::string("    "), "<mask>", "</mask>\n");
    out << content_.get()->tostring_part(indent + std::string("    "), "<content>", "</content>\n");
    out << indent << "</" << classname() << ">" << post;
    return out.str();
  }

  void ByteMaskedArray::tojson_part(ToJson& builder) const {
    int64_t len = length();
    check_for_iteration();
    builder.beginlist();
    for (int64_t i = 0;  i < len;  i++) {
      getitem_at_nowrap(i).get()->tojson_part(builder);
    }
    builder.endlist();
  }

  void ByteMaskedArray::nbytes_part(std::map<size_t, int64_t>& largest) const {
    mask_.nbytes_part(largest);
    content_.get()->nbytes_part(largest);
    if (identities_.get() != nullptr) {
      identities_.get()->nbytes_part(largest);
    }
  }

  int64_t ByteMaskedArray::length() const {
    return mask_.length();
  }

  const std::shared_ptr<Content> ByteMaskedArray::shallow_copy() const {
    return std::make_shared<ByteMaskedArray>(identities_, parameters_, mask_, content_, validwhen_);
  }

  const std::shared_ptr<Content> ByteMaskedArray::deep_copy(bool copyarrays, bool copyindexes, bool copyidentities) const {
    throw std::runtime_error("FIXME: ByteMaskedArray::deep_copy");
  }

  void ByteMaskedArray::check_for_iteration() const {
    if (identities_.get() != nullptr  &&  identities_.get()->length() < length()) {
      util::handle_error(failure("len(identities) < len(array)", kSliceNone, kSliceNone), identities_.get()->classname(), nullptr);
    }
  }

  const std::shared_ptr<Content> ByteMaskedArray::getitem_nothing() const {
    throw std::runtime_error("FIXME: ByteMaskedArray::getitem_nothing");
  }

  const std::shared_ptr<Content> ByteMaskedArray::getitem_at(int64_t at) const {
    int64_t regular_at = at;
    if (regular_at < 0) {
      regular_at += length();
    }
    if (!(0 <= regular_at  &&  regular_at < length())) {
      util::handle_error(failure("index out of range", kSliceNone, at), classname(), identities_.get());
    }
    return getitem_at_nowrap(regular_at);
  }

  const std::shared_ptr<Content> ByteMaskedArray::getitem_at_nowrap(int64_t at) const {
    bool msk = (mask_.getitem_at_nowrap(at) != 0);
    if (msk == validwhen_) {
      return content_.get()->getitem_at_nowrap(at);
    }
    else {
      return none;
    }
  }

  const std::shared_ptr<Content> ByteMaskedArray::getitem_range(int64_t start, int64_t stop) const {
    int64_t regular_start = start;
    int64_t regular_stop = stop;
    awkward_regularize_rangeslice(&regular_start, &regular_stop, true, start != Slice::none(), stop != Slice::none(), length());
    if (identities_.get() != nullptr  &&  regular_stop > identities_.get()->length()) {
      util::handle_error(failure("index out of range", kSliceNone, stop), identities_.get()->classname(), nullptr);
    }
    return getitem_range_nowrap(regular_start, regular_stop);
  }

  const std::shared_ptr<Content> ByteMaskedArray::getitem_range_nowrap(int64_t start, int64_t stop) const {
    std::shared_ptr<Identities> identities(nullptr);
    if (identities_.get() != nullptr) {
      identities = identities_.get()->getitem_range_nowrap(start, stop);
    }
    return std::make_shared<ByteMaskedArray>(identities, parameters_, mask_.getitem_range_nowrap(start, stop), content_.get()->getitem_range_nowrap(start, stop), validwhen_);
  }

  const std::shared_ptr<Content> ByteMaskedArray::getitem_field(const std::string& key) const {
    return std::make_shared<ByteMaskedArray>(identities_, util::Parameters(), mask_, content_.get()->getitem_field(key), validwhen_);
  }

  const std::shared_ptr<Content> ByteMaskedArray::getitem_fields(const std::vector<std::string>& keys) const {
    return std::make_shared<ByteMaskedArray>(identities_, util::Parameters(), mask_, content_.get()->getitem_fields(keys), validwhen_);
  }

  const std::shared_ptr<Content> ByteMaskedArray::getitem_next(const std::shared_ptr<SliceItem>& head, const Slice& tail, const Index64& advanced) const {
    if (head.get() == nullptr) {
      return shallow_copy();
    }
    else if (dynamic_cast<SliceAt*>(head.get())  ||  dynamic_cast<SliceRange*>(head.get())  ||  dynamic_cast<SliceArray64*>(head.get())  ||  dynamic_cast<SliceJagged64*>(head.get())) {
      int64_t numnull;
      std::pair<Index64, Index64> pair = nextcarry_outindex(numnull);
      Index64 nextcarry = pair.first;
      Index64 outindex = pair.second;

      std::shared_ptr<Content> next = content_.get()->carry(nextcarry);

      std::shared_ptr<Content> out = next.get()->getitem_next(head, tail, advanced);
      IndexedOptionArray64 out2(identities_, parameters_, outindex, out);
      return out2.simplify();
    }
    else if (SliceEllipsis* ellipsis = dynamic_cast<SliceEllipsis*>(head.get())) {
      return Content::getitem_next(*ellipsis, tail, advanced);
    }
    else if (SliceNewAxis* newaxis = dynamic_cast<SliceNewAxis*>(head.get())) {
      return Content::getitem_next(*newaxis, tail, advanced);
    }
    else if (SliceField* field = dynamic_cast<SliceField*>(head.get())) {
      return Content::getitem_next(*field, tail, advanced);
    }
    else if (SliceFields* fields = dynamic_cast<SliceFields*>(head.get())) {
      return Content::getitem_next(*fields, tail, advanced);
    }
    else if (SliceMissing64* missing = dynamic_cast<SliceMissing64*>(head.get())) {
      return Content::getitem_next(*missing, tail, advanced);
    }
    else {
      throw std::runtime_error("unrecognized slice type");
    }
  }

  const std::shared_ptr<Content> ByteMaskedArray::carry(const Index64& carry) const {
    Index8 nextmask(carry.length());
    struct Error err = awkward_bytemaskedarray_getitem_carry_64(
      nextmask.ptr().get(),
      mask_.ptr().get(),
      mask_.offset(),
      mask_.length(),
      carry.ptr().get(),
      carry.length());
    util::handle_error(err, classname(), identities_.get());
    std::shared_ptr<Identities> identities(nullptr);
    if (identities_.get() != nullptr) {
      identities = identities_.get()->getitem_carry_64(carry);
    }
    return std::make_shared<ByteMaskedArray>(identities, parameters_, nextmask, content_.get()->carry(carry), validwhen_);
  }

  const std::string ByteMaskedArray::purelist_parameter(const std::string& key) const {
    std::string out = parameter(key);
    if (out == std::string("null")) {
      return content_.get()->purelist_parameter(key);
    }
    else {
      return out;
    }
  }

  bool ByteMaskedArray::purelist_isregular() const {
    return content_.get()->purelist_isregular();
  }

  int64_t ByteMaskedArray::purelist_depth() const {
    return content_.get()->purelist_depth();
  }

  const std::pair<int64_t, int64_t> ByteMaskedArray::minmax_depth() const {
    return content_.get()->minmax_depth();
  }

  const std::pair<bool, int64_t> ByteMaskedArray::branch_depth() const {
    return content_.get()->branch_depth();
  }

  int64_t ByteMaskedArray::numfields() const {
    return content_.get()->numfields();
  }

  int64_t ByteMaskedArray::fieldindex(const std::string& key) const {
    return content_.get()->fieldindex(key);
  }

  const std::string ByteMaskedArray::key(int64_t fieldindex) const {
    return content_.get()->key(fieldindex);
  }

  bool ByteMaskedArray::haskey(const std::string& key) const {
    return content_.get()->haskey(key);
  }

  const std::vector<std::string> ByteMaskedArray::keys() const {
    return content_.get()->keys();
  }

  const std::string ByteMaskedArray::validityerror(const std::string& path) const {
    throw std::runtime_error("FIXME: ByteMaskedArray::validityerror");
  }

  const std::shared_ptr<Content> ByteMaskedArray::num(int64_t axis, int64_t depth) const {
    throw std::runtime_error("FIXME: ByteMaskedArray::num");
  }

  const std::pair<Index64, std::shared_ptr<Content>> ByteMaskedArray::offsets_and_flattened(int64_t axis, int64_t depth) const {
    throw std::runtime_error("FIXME: ByteMaskedArray::offsets_and_flattened");
  }

  bool ByteMaskedArray::mergeable(const std::shared_ptr<Content>& other, bool mergebool) const {
    throw std::runtime_error("FIXME: ByteMaskedArray::mergeable");
  }

  const std::shared_ptr<Content> ByteMaskedArray::reverse_merge(const std::shared_ptr<Content>& other) const {
    throw std::runtime_error("FIXME: ByteMaskedArray::reverse_merge");
  }

  const std::shared_ptr<Content> ByteMaskedArray::merge(const std::shared_ptr<Content>& other) const {
    throw std::runtime_error("FIXME: ByteMaskedArray::merge");
  }

  const std::shared_ptr<SliceItem> ByteMaskedArray::asslice() const {
    return toIndexedOptionArray64().get()->asslice();
  }

  const std::shared_ptr<Content> ByteMaskedArray::rpad(int64_t target, int64_t axis, int64_t depth) const {
    throw std::runtime_error("FIXME: ByteMaskedArray::rpad");
  }

  const std::shared_ptr<Content> ByteMaskedArray::rpad_and_clip(int64_t target, int64_t axis, int64_t depth) const {
    throw std::runtime_error("FIXME: ByteMaskedArray::rpad_and_clip");
  }

  const std::shared_ptr<Content> ByteMaskedArray::reduce_next(const Reducer& reducer, int64_t negaxis, const Index64& parents, int64_t outlength, bool mask, bool keepdims) const {
    throw std::runtime_error("FIXME: ByteMaskedArray::reduce_next");
  }

  const std::shared_ptr<Content> ByteMaskedArray::localindex(int64_t axis, int64_t depth) const {
    throw std::runtime_error("FIXME: ByteMaskedArray::localindex");
  }

  const std::shared_ptr<Content> ByteMaskedArray::choose(int64_t n, bool diagonal, const std::shared_ptr<util::RecordLookup>& recordlookup, const util::Parameters& parameters, int64_t axis, int64_t depth) const {
    throw std::runtime_error("FIXME: ByteMaskedArray::choose");
  }

  const std::shared_ptr<Content> ByteMaskedArray::getitem_next(const SliceAt& at, const Slice& tail, const Index64& advanced) const {
    throw std::runtime_error("undefined operation: IndexedArray::getitem_next(at)");
  }

  const std::shared_ptr<Content> ByteMaskedArray::getitem_next(const SliceRange& range, const Slice& tail, const Index64& advanced) const {
    throw std::runtime_error("undefined operation: IndexedArray::getitem_next(range)");
  }

  const std::shared_ptr<Content> ByteMaskedArray::getitem_next(const SliceArray64& array, const Slice& tail, const Index64& advanced) const {
    throw std::runtime_error("undefined operation: IndexedArray::getitem_next(array)");
  }

  const std::shared_ptr<Content> ByteMaskedArray::getitem_next(const SliceJagged64& jagged, const Slice& tail, const Index64& advanced) const {
    throw std::runtime_error("undefined operation: IndexedArray::getitem_next(jagged)");
  }

  const std::shared_ptr<Content> ByteMaskedArray::getitem_next_jagged(const Index64& slicestarts, const Index64& slicestops, const SliceArray64& slicecontent, const Slice& tail) const {
    return getitem_next_jagged_generic<SliceArray64>(slicestarts, slicestops, slicecontent, tail);
  }

  const std::shared_ptr<Content> ByteMaskedArray::getitem_next_jagged(const Index64& slicestarts, const Index64& slicestops, const SliceMissing64& slicecontent, const Slice& tail) const {
    return getitem_next_jagged_generic<SliceMissing64>(slicestarts, slicestops, slicecontent, tail);
  }

  const std::shared_ptr<Content> ByteMaskedArray::getitem_next_jagged(const Index64& slicestarts, const Index64& slicestops, const SliceJagged64& slicecontent, const Slice& tail) const {
    return getitem_next_jagged_generic<SliceJagged64>(slicestarts, slicestops, slicecontent, tail);
  }

  template <typename S>
  const std::shared_ptr<Content> ByteMaskedArray::getitem_next_jagged_generic(const Index64& slicestarts, const Index64& slicestops, const S& slicecontent, const Slice& tail) const {
    throw std::runtime_error("FIXME: ByteMaskedArray::getitem_next_jagged_generic");
  }

  const std::pair<Index64, Index64> ByteMaskedArray::nextcarry_outindex(int64_t& numnull) const {
    struct Error err1 = awkward_bytemaskedarray_numnull(
      &numnull,
      mask_.ptr().get(),
      mask_.offset(),
      mask_.length(),
      validwhen_);
    util::handle_error(err1, classname(), identities_.get());

    Index64 nextcarry(length() - numnull);
    Index64 outindex(length());
    struct Error err2 = awkward_bytemaskedarray_getitem_nextcarry_outindex_64(
      nextcarry.ptr().get(),
      outindex.ptr().get(),
      mask_.ptr().get(),
      mask_.offset(),
      mask_.length(),
      validwhen_);
    util::handle_error(err2, classname(), identities_.get());

    return std::pair<Index64, Index64>(nextcarry, outindex);
  }

}