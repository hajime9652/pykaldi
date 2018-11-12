#ifndef PYKALDI_HMM_POSTERIOR_EXT_H_
#define PYKALDI_HMM_POSTERIOR_EXT_H_ 1

#include "hmm/posterior.h"

namespace kaldi {

class FramePosterior {
 public:
  FramePosterior() {}

  // FramePosterior(const FramePosterior &other) : post_(other.post_) {}
  //
  // FramePosterior(FramePosterior &&other)
  //   : post_(std::move(other.post_)) {}

  explicit FramePosterior(const Posterior &post) : post_(post) {}

  explicit FramePosterior(const std::vector<int32> &ali) : post_(ali.size()) {
    AlignmentToPosterior(ali, &post_);
  }

  const Posterior & GetPosterior() const {
    return post_;
  }

  Posterior * GetMutablePosterior() {
    return &post_;
  }

  void Write(std::ostream &os, bool binary) const {
    WritePosterior(os, binary, post_);
  }

  void Read(std::istream &os, bool binary) {
    ReadPosterior(os, binary, &post_);
  }

  void Scale(BaseFloat scale) {
    ScalePosterior(scale, &post_);
  }

  BaseFloat Total() const {
    return TotalPosterior(post_);
  }

  void SortByPdfs(const TransitionModel &tmodel) {
    SortPosteriorByPdfs(tmodel, &post_);
  }

  void WeightSilence(const TransitionModel &tmodel,
                     const ConstIntegerSet<int32> &silence_set,
                     BaseFloat silence_scale) {
    WeightSilencePost(tmodel, silence_set, silence_scale, &post_);
  }

  void WeightSilenceDistributed(const TransitionModel &tmodel,
                                const ConstIntegerSet<int32> &silence_set,
                                BaseFloat silence_scale) {
    WeightSilencePostDistributed(tmodel, silence_set, silence_scale, &post_);
  }

  template <typename Real>
  void ToMatrix(const int32 post_dim, Matrix<Real> *mat) const {
    PosteriorToMatrix(post_, post_dim, mat);
  }

  template <typename Real>
  void ToPdfMatrix(const TransitionModel &tmodel, Matrix<Real> *mat) const {
    PosteriorToPdfMatrix(post_, tmodel, mat);
  }

  void ConvertTransitionsToPdfs(const TransitionModel &tmodel,
                                FramePosterior *post_pdf) const {
    ConvertPosteriorToPdfs(tmodel, post_, post_pdf->GetMutablePosterior());
  }

  void ConvertTransitionsToPhones(const TransitionModel &tmodel,
                                  FramePosterior *post_phone) const {
    ConvertPosteriorToPhones(tmodel, post_, post_phone->GetMutablePosterior());
  }

 private:
   Posterior post_;
};

int32 MergeFramePosteriors(const FramePosterior & post1,
                           const FramePosterior & post2,
                           bool merge,
                           bool drop_frames,
                           FramePosterior *post_out) {
  return MergePosteriors(post1.GetPosterior(), post2.GetPosterior(),
                         merge, drop_frames, post_out->GetMutablePosterior());
}


class FramePosteriorHolder {
 public:
  typedef FramePosterior T;

  FramePosteriorHolder() { }

  static bool Write(std::ostream &os, bool binary, const T &t) {
    InitKaldiOutputStream(os, binary);  // Puts binary header if binary mode.
    try {
      t.Write(os, binary);
      return true;
    } catch(const std::exception &e) {
      KALDI_WARN << "Exception caught writing table of posteriors. " << e.what();
      return false;  // Write failure.
    }
  }

  void Clear() { FramePosterior tmp; std::swap(tmp, t_); }

  // Reads into the holder.
  bool Read(std::istream &is) {
    Clear();

    bool is_binary;
    if (!InitKaldiInputStream(is, &is_binary)) {
      KALDI_WARN << "Reading Table object, failed reading binary header";
      return false;
    }
    try {
      t_.Read(is, is_binary);
      return true;
    } catch (std::exception &e) {
      KALDI_WARN << "Exception caught reading table of posteriors. " << e.what();
      Clear();
      return false;
    }
  }

  // Kaldi objects always have the stream open in binary mode for
  // reading.
  static bool IsReadInBinary() { return true; }

  T &Value() { return t_; }

  void Swap(FramePosteriorHolder *other) {
    std::swap(t_, other->t_);
  }

  bool ExtractRange(const FramePosteriorHolder &other, const std::string &range) {
    KALDI_ERR << "ExtractRange is not defined for this type of holder.";
    return false;
  }
 private:
  KALDI_DISALLOW_COPY_AND_ASSIGN(FramePosteriorHolder);
  T t_;
};

}

#endif  // PYKALDI_HMM_POSTERIOR_EXT_H_
