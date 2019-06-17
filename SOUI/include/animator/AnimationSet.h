/*
* Copyright (C) 2006 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#pragma once
#include <animator/Animation.h>
#include <souicoll.h>

/**
* Represents a group of Animations that should be played together.
* The transformation of each individual animation are composed 
* together into a single transform. 
* If AnimationSet sets any properties that its children also set
* (for example, duration or fillBefore), the values of AnimationSet
* override the child values.
*
* <p>The way that AnimationSet inherits behavior from Animation is important to
* understand. Some of the Animation attributes applied to AnimationSet affect the
* AnimationSet itself, some are pushed down to the children, and some are ignored,
* as follows:
* <ul>
*     <li>duration, repeatMode, fillBefore, fillAfter: These properties, when set
*     on an AnimationSet object, will be pushed down to all child animations.</li>
*     <li>repeatCount, fillEnabled: These properties are ignored for AnimationSet.</li>
*     <li>startOffset, shareInterpolator: These properties apply to the AnimationSet itself.</li>
* </ul>
* Starting with {@link android.os.Build.VERSION_CODES#ICE_CREAM_SANDWICH},
* the behavior of these properties is the same in XML resources and at runtime (prior to that
* release, the values set in XML were ignored for AnimationSet). That is, calling
* <code>setDuration(500)</code> on an AnimationSet has the same effect as declaring
* <code>android:duration="500"</code> in an XML resource for an AnimationSet object.</p>
*/

namespace SOUI{

	class AnimationSet : public Animation {
		enum{
			PROPERTY_FILL_AFTER_MASK         = 0x1,
			PROPERTY_FILL_BEFORE_MASK        = 0x2,
			PROPERTY_REPEAT_MODE_MASK        = 0x4,
			PROPERTY_START_OFFSET_MASK       = 0x8,
			PROPERTY_SHARE_INTERPOLATOR_MASK = 0x10,
			PROPERTY_DURATION_MASK           = 0x20,
			PROPERTY_MORPH_MATRIX_MASK       = 0x40,
			PROPERTY_CHANGE_BOUNDS_MASK      = 0x80,
		};

	private:
		int mFlags = 0;
		boolean mDirty;
		boolean mHasAlpha;

		SArray<Animation> mAnimations;

		Transformation  mTempTransformation;

		long mLastEnd;

		/**
		* Constructor to use when building an AnimationSet from code
		* 
		* @param shareInterpolator Pass true if all of the animations in this set
		*        should use the interpolator associated with this AnimationSet.
		*        Pass false if each animation should use its own interpolator.
		*/
	public: AnimationSet(boolean shareInterpolator) {
				setFlag(PROPERTY_SHARE_INTERPOLATOR_MASK, shareInterpolator);
				init();
			}

			//protected AnimationSet clone() {
			//    AnimationSet animation = (AnimationSet) Animation::clone();
			//    animation.mTempTransformation = new Transformation();
			//    animation.mAnimations = new ArrayList<Animation>();

			//    int count = mAnimations.size();
			//    ArrayList<Animation> animations = mAnimations;

			//    for (int i = 0; i < count; i++) {
			//        animation.mAnimations.add(animations.get(i).clone());
			//    }

			//    return animation;
			//}

	private: void setFlag(int mask, boolean value) {
				 if (value) {
					 mFlags |= mask;
				 } else {
					 mFlags &= ~mask;
				 }
			 }

	private: void init() {
				 mStartTime = 0;
			 }

	public: void setFillAfter(boolean fillAfter) {
				mFlags |= PROPERTY_FILL_AFTER_MASK;
				Animation::setFillAfter(fillAfter);
			}

	public: void setFillBefore(boolean fillBefore) {
				mFlags |= PROPERTY_FILL_BEFORE_MASK;
				Animation::setFillBefore(fillBefore);
			}

	public: void setRepeatMode(int repeatMode) {
				mFlags |= PROPERTY_REPEAT_MODE_MASK;
				Animation::setRepeatMode(repeatMode);
			}

	public: void setStartOffset(long startOffset) {
				mFlags |= PROPERTY_START_OFFSET_MASK;
				Animation::setStartOffset(startOffset);
			}

			/**
			* @hide
			*/
	public: boolean hasAlpha() {
				if (mDirty) {
					mDirty = mHasAlpha = false;

					int count = mAnimations.GetCount();
					for (int i = 0; i < count; i++) {
						if (mAnimations.GetAt(i).hasAlpha()) {
							mHasAlpha = true;
							break;
						}
					}
				}

				return mHasAlpha;
			}

			/**
			* <p>Sets the duration of every child animation.</p>
			*
			* @param durationMillis the duration of the animation, in milliseconds, for
			*        every child in this set
			*/
	public: void setDuration(long durationMillis) {
				mFlags |= PROPERTY_DURATION_MASK;
				Animation::setDuration(durationMillis);
				mLastEnd = mStartOffset + mDuration;
			}

			/**
			* Add a child animation to this animation set.
			* The transforms of the child animations are applied in the order
			* that they were added
			* @param a Animation to add.
			*/
	public: void addAnimation(Animation  a) {
				mAnimations.Add(a);

				if ((mFlags & PROPERTY_DURATION_MASK) == PROPERTY_DURATION_MASK) {
					mLastEnd = mStartOffset + mDuration;
				} else {
					if (mAnimations.GetCount() == 1) {
						mDuration = a.getStartOffset() + a.getDuration();
						mLastEnd = mStartOffset + mDuration;
					} else {
						mLastEnd = smax(mLastEnd, a.getStartOffset() + a.getDuration());
						mDuration = mLastEnd - mStartOffset;
					}
				}

				mDirty = true;
			}

			/**
			* Sets the start time of this animation and all child animations
			* 
			* @see android.view.animation.Animation#setStartTime(long)
			*/
	public: void setStartTime(long startTimeMillis) {
				Animation::setStartTime(startTimeMillis);

				int count = mAnimations.GetCount();

				for (int i = 0; i < count; i++) {
					Animation & a = mAnimations[i];
					a.setStartTime(startTimeMillis);
				}
			}

	public: long getStartTime() {
				long startTime = 100000;

				int count = mAnimations.GetCount();

				for (int i = 0; i < count; i++) {
					Animation &a = mAnimations.GetAt(i);
					startTime = smin(startTime, a.getStartTime());
				}

				return startTime;
			}

			/**
			* The duration of an AnimationSet is defined to be the 
			* duration of the longest child animation.
			* 
			* @see android.view.animation.Animation#getDuration()
			*/
	public: long getDuration() {
				int count = mAnimations.GetCount();
				long duration = 0;

				boolean durationSet = (mFlags & PROPERTY_DURATION_MASK) == PROPERTY_DURATION_MASK;
				if (durationSet) {
					duration = mDuration;
				} else {
					for (int i = 0; i < count; i++) {
						duration = smax(duration, mAnimations[i].getDuration());
					}
				}

				return duration;
			}

			/**
			* The duration hint of an animation set is the maximum of the duration
			* hints of all of its component animations.
			* 
			* @see android.view.animation.Animation#computeDurationHint
			*/
	public: long computeDurationHint() {
				long duration = 0;
				int count = mAnimations.GetCount();
				for (int i = count - 1; i >= 0; --i) {
					long d = mAnimations[i].computeDurationHint();
					if (d > duration) duration = d;
				}
				return duration;
			}



			/**
			* The transformation of an animation set is the concatenation of all of its
			* component animations.
			* 
			* @see android.view.animation.Animation#getTransformation
			*/
	public: boolean getTransformation(long currentTime, Transformation &t) {
				int count = mAnimations.GetCount();
				Transformation temp = mTempTransformation;

				boolean more = false;
				boolean started = false;
				boolean ended = true;

				t.clear();

				for (int i = count - 1; i >= 0; --i) {
					Animation & a = mAnimations[i];

					temp.clear();
					more = a.getTransformation(currentTime, temp, getScaleFactor()) || more;
					t.compose(temp);

					started = started || a.hasStarted();
					ended = a.hasEnded() && ended;
				}

				if (started && !mStarted) {
					if (mListener != NULL) {
						mListener->onAnimationStart(this);
					}
					mStarted = true;
				}

				if (ended != mEnded) {
					if (mListener != NULL) {
						mListener->onAnimationEnd(this);
					}
					mEnded = ended;
				}

				return more;
			}

			/**
			* @see android.view.animation.Animation#scaleCurrentDuration(float)
			*/
	public: void scaleCurrentDuration(float scale) {
				int count = mAnimations.GetCount();
				for (int i = 0; i < count; i++) {
					mAnimations[i].scaleCurrentDuration(scale);
				}
			}

			/**
			* @see android.view.animation.Animation#initialize(int, int, int, int)
			*/
	public: void initialize(int width, int height, int parentWidth, int parentHeight) {
				//        Animation::initialize(width, height, parentWidth, parentHeight);

				boolean durationSet = (mFlags & PROPERTY_DURATION_MASK) == PROPERTY_DURATION_MASK;
				boolean fillAfterSet = (mFlags & PROPERTY_FILL_AFTER_MASK) == PROPERTY_FILL_AFTER_MASK;
				boolean fillBeforeSet = (mFlags & PROPERTY_FILL_BEFORE_MASK) == PROPERTY_FILL_BEFORE_MASK;
				boolean repeatModeSet = (mFlags & PROPERTY_REPEAT_MODE_MASK) == PROPERTY_REPEAT_MODE_MASK;
				boolean shareInterpolator = (mFlags & PROPERTY_SHARE_INTERPOLATOR_MASK)
					== PROPERTY_SHARE_INTERPOLATOR_MASK;
				boolean startOffsetSet = (mFlags & PROPERTY_START_OFFSET_MASK)
					== PROPERTY_START_OFFSET_MASK;

				if (shareInterpolator) {
					ensureInterpolator();
				}

				//ArrayList<Animation> children = mAnimations;
				//int count = children.size();

				//long duration = mDuration;
				//boolean fillAfter = mFillAfter;
				//boolean fillBefore = mFillBefore;
				//int repeatMode = mRepeatMode;
				//Interpolator interpolator = mInterpolator;
				//long startOffset = mStartOffset;


				//long[] storedOffsets = mStoredOffsets;
				//if (startOffsetSet) {
				//    if (storedOffsets == null || storedOffsets.length != count) {
				//        storedOffsets = mStoredOffsets = new long[count];
				//    }
				//} else if (storedOffsets != null) {
				//    storedOffsets = mStoredOffsets = null;
				//}

				//for (int i = 0; i < count; i++) {
				//    Animation a = children.get(i);
				//    if (durationSet) {
				//        a.setDuration(duration);
				//    }
				//    if (fillAfterSet) {
				//        a.setFillAfter(fillAfter);
				//    }
				//    if (fillBeforeSet) {
				//        a.setFillBefore(fillBefore);
				//    }
				//    if (repeatModeSet) {
				//        a.setRepeatMode(repeatMode);
				//    }
				//    if (shareInterpolator) {
				//        a.setInterpolator(interpolator);
				//    }
				//    if (startOffsetSet) {
				//        long offset = a.getStartOffset();
				//        a.setStartOffset(offset + startOffset);
				//        storedOffsets[i] = offset;
				//    }
				//    a.initialize(width, height, parentWidth, parentHeight);
				//}
			}

	public: void reset() {
				Animation::reset();
				restoreChildrenStartOffset();
			}

			/**
			* @hide
			*/
			void restoreChildrenStartOffset() {
				//long[] offsets = mStoredOffsets;
				//if (offsets == NULL) return;

				//ArrayList<Animation> children = mAnimations;
				//int count = children.size();

				//for (int i = 0; i < count; i++) {
				//    children.get(i).setStartOffset(offsets[i]);
				//}
			}

	};

}