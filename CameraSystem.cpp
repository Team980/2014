#include "WPILib.h"
#include "Constants.h"
#include "Math.h"

extern void message(char *fmt, ...);

#define DEBUG 0


class CameraSystem {
	public:
	
		struct Scores {
			double rectangularity;
			double aspectRatioVertical;
			double aspectRatioHorizontal;
		};
		
		struct TargetReport {
			int verticalIndex;
			int horizontalIndex;
			bool Hot;
			double totalScore;
			double leftScore;
			double rightScore;
			double tapeWidthScore;
			double verticalScore;
		};
	
		CameraSystem(AxisCamera &c) : camera(c)
		{
			
		}
		
//		CameraSystem(AxisCamera &c) : camera(c) {
//			c = AxisCamera::GetInstance();	//To use the Axis camera uncomment this line
//		}
		
		float GetDistance() {
			return distance;
		}
		
		
		bool HotOrNot() {
			return hot;
		}
			
		void Scan() {
			Scores *scores;
			TargetReport target;
			int verticalTargets[MAX_PARTICLES];
			int horizontalTargets[MAX_PARTICLES];
			int verticalTargetCount, horizontalTargetCount;
			Threshold threshold(0, 255, 50, 255, 70, 255);
			ParticleFilterCriteria2 criteria[] = {{IMAQ_MT_AREA, AREA_MINIMUM, 65535, false, false}};
			ColorImage *image;

//			AxisCamera &camera = AxisCamera::GetInstance();	//To use the Axis camera uncomment this line

			image = camera.GetImage();
#if DEBUG
			image->Write("/raw.bmp");
#endif
			BinaryImage *thresholdImage = image->ThresholdHSV(threshold);
#if DEBUG
			thresholdImage->Write("/threshold.bmp");
#endif
			BinaryImage *filteredImage = thresholdImage->ParticleFilter(criteria, 1);
#if DEBUG
			filteredImage->Write("Filtered.bmp");
#endif
			
			vector<ParticleAnalysisReport> *reports = filteredImage->GetOrderedParticleAnalysisReports();
			verticalTargetCount = horizontalTargetCount = 0;
#if DEBUG
			message("reports->size = %d", reports->size());
#endif
			if (reports->size() > 0) {
				scores = new Scores[reports->size()];
				for (unsigned int i = 0; i < MAX_PARTICLES && i < reports->size(); i++) {
					ParticleAnalysisReport *report = &(reports->at(i));
					//Score each particle on rectangularity and aspect ratio
					scores[i].rectangularity = scoreRectangularity(report);
					scores[i].aspectRatioVertical = scoreAspectRatio(filteredImage, report, true);
					scores[i].aspectRatioHorizontal = scoreAspectRatio(filteredImage, report, false);
#if 0					
					if (scoreCompare(scores[i], false)) {
						horizontalTargets[horizontalTargetCount++] = i;
					} else if (scoreCompare(scores[i], true)) {
						verticalTargets[verticalTargetCount++] = i;
					}
#endif					
					//Check if the particle is a horizontal target, if not, check if it's a vertical target
					if(scoreCompare(scores[i], false))
					{
#if DEBUG
						message("particle: %d  is a Horizontal Target centerX: %d  centerY: %d \n", i, report->center_mass_x, report->center_mass_y);
#endif
						horizontalTargets[horizontalTargetCount++] = i; //Add particle to target array and increment count
					} else if (scoreCompare(scores[i], true)) {
#if DEBUG
						message("particle: %d  is a Vertical Target centerX: %d  centerY: %d \n", i, report->center_mass_x, report->center_mass_y);
#endif
						verticalTargets[verticalTargetCount++] = i;  //Add particle to target array and increment count
					} else {
#if DEBUG
						message("particle: %d  is not a Target centerX: %d  centerY: %d \n", i, report->center_mass_x, report->center_mass_y);
#endif
						}
#if DEBUG
					message("Scores rect: %f  ARvert: %f \n", scores[i].rectangularity, scores[i].aspectRatioVertical);
					message("ARhoriz: %f  \n", scores[i].aspectRatioHorizontal);	
#endif					
					
					
				}
				
				target.totalScore = target.leftScore = target.rightScore = target.tapeWidthScore = target.verticalScore = 0;
				target.verticalIndex = verticalTargets[0];
				for (int i = 0; i < verticalTargetCount; i++) {
					ParticleAnalysisReport *verticalReport = &(reports->at(verticalTargets[i]));
					for (int j = 0; j < horizontalTargetCount; j++) {
						ParticleAnalysisReport *horizontalReport = &(reports->at(horizontalTargets[j]));
						double horizWidth, horizHeight, vertWidth, leftScore, rightScore, tapeWidthScore, verticalScore, total;
						imaqMeasureParticle(filteredImage->GetImaqImage(), horizontalReport->particleIndex, 0, IMAQ_MT_EQUIVALENT_RECT_LONG_SIDE, &horizWidth);
						imaqMeasureParticle(filteredImage->GetImaqImage(), verticalReport->particleIndex, 0, IMAQ_MT_EQUIVALENT_RECT_SHORT_SIDE, &vertWidth);
						imaqMeasureParticle(filteredImage->GetImaqImage(), horizontalReport->particleIndex, 0, IMAQ_MT_EQUIVALENT_RECT_SHORT_SIDE, &horizHeight);

						leftScore = ratioToScore(1.2*(verticalReport->boundingRect.left - horizontalReport->center_mass_x)/horizWidth);
						rightScore = ratioToScore(1.2*(horizontalReport->center_mass_x - verticalReport->boundingRect.left - verticalReport->boundingRect.width)/horizWidth);
						tapeWidthScore = ratioToScore(vertWidth/horizHeight);
						verticalScore = ratioToScore(1-(verticalReport->boundingRect.top - horizontalReport->center_mass_y)/(4*horizHeight));
						total = leftScore > rightScore ? leftScore:rightScore;
						total += tapeWidthScore + verticalScore;

						if(total > target.totalScore) {
							target.horizontalIndex = horizontalTargets[j];
							target.verticalIndex = verticalTargets[i];
							target.totalScore = total;
							target.leftScore = leftScore;
							target.rightScore = rightScore;
							target.tapeWidthScore = tapeWidthScore;
							target.verticalScore = verticalScore;
						}
					}
					target.Hot = hotOrNot(target);

					if(verticalTargetCount > 0) {
						ParticleAnalysisReport *distanceReport = &(reports->at(target.verticalIndex));
						distance = computeDistance(filteredImage, distanceReport);
						hot = target.Hot;
						message("mll: hot = %d", hot);
						message("mll: distance = %f", distance);
					}
				}
			}
		}

	
	
	private:
		bool hot;
		float distance;
		AxisCamera &camera;

		double computeDistance (BinaryImage *image, ParticleAnalysisReport *report) {
			double rectLong, height;
			int targetHeight;

			imaqMeasureParticle(image->GetImaqImage(), report->particleIndex, 0, IMAQ_MT_EQUIVALENT_RECT_LONG_SIDE, &rectLong);
			//using the smaller of the estimated rectangle long side and the bounding rectangle height results in better performance
			//on skewed rectangles
			height = min(report->boundingRect.height, rectLong);
			targetHeight = 32;

			return Y_IMAGE_RES * targetHeight / (height * 12 * 2 * tan(VIEW_ANGLE*PI/(180*2)));
		}
		double scoreAspectRatio(BinaryImage *image, ParticleAnalysisReport *report, bool vertical) {
			double rectLong, rectShort, idealAspectRatio, aspectRatio;
			idealAspectRatio = vertical ? (4.0/32) : (23.5/4);	//Vertical reflector 4" wide x 32" tall, horizontal 23.5" wide x 4" tall

			imaqMeasureParticle(image->GetImaqImage(), report->particleIndex, 0, IMAQ_MT_EQUIVALENT_RECT_LONG_SIDE, &rectLong);
			imaqMeasureParticle(image->GetImaqImage(), report->particleIndex, 0, IMAQ_MT_EQUIVALENT_RECT_SHORT_SIDE, &rectShort);

			//Divide width by height to measure aspect ratio
			if(report->boundingRect.width > report->boundingRect.height) {
				//particle is wider than it is tall, divide long by short
				aspectRatio = ratioToScore(((rectLong/rectShort)/idealAspectRatio));
			} else {
				//particle is taller than it is wide, divide short by long
				aspectRatio = ratioToScore(((rectShort/rectLong)/idealAspectRatio));
			}
			return aspectRatio;		//force to be in range 0-100
		}
		bool scoreCompare(Scores scores, bool vertical) {
			bool isTarget = true;

			isTarget &= scores.rectangularity > RECTANGULARITY_LIMIT;
			if(vertical){
				isTarget &= scores.aspectRatioVertical > ASPECT_RATIO_LIMIT;
			} else {
				isTarget &= scores.aspectRatioHorizontal > ASPECT_RATIO_LIMIT;
			}

			return isTarget;
		}
		double scoreRectangularity(ParticleAnalysisReport *report) {
			if(report->boundingRect.width*report->boundingRect.height !=0){
				return 100*report->particleArea/(report->boundingRect.width*report->boundingRect.height);
			} else {
				return 0;
			}	
		}
		double ratioToScore(double ratio) {
			return (max(0, min(100*(1-fabs(1-ratio)), 100)));
		}
		bool hotOrNot(TargetReport target) {
			bool isHot = true;

			isHot &= target.tapeWidthScore >= TAPE_WIDTH_LIMIT;
			isHot &= target.verticalScore >= VERTICAL_SCORE_LIMIT;
			isHot &= (target.leftScore > LR_SCORE_LIMIT) | (target.rightScore > LR_SCORE_LIMIT);

			return isHot;
		}
};
