#include "Classifier.h"

/**
* Predict class for features vector
*/
int predict(float *x) {
    uint8_t votes[5] = { 0 };
    // tree #1
    if (x[4] <= 533.5) {
        if (x[7] <= 870.5) {
            if (x[7] <= 840.0) {
                if (x[6] <= 556.5) {
                    if (x[2] <= 7.5) {
                        if (x[3] <= 37.5) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[6] <= 546.0) {
                                votes[3] += 1;
                            }

                            else {
                                votes[1] += 1;
                            }
                        }
                    }

                    else {
                        if (x[5] <= 334.5) {
                            votes[1] += 1;
                        }

                        else {
                            if (x[3] <= 43.5) {
                                if (x[4] <= 148.0) {
                                    if (x[5] <= 335.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    if (x[2] <= 11.5) {
                                        if (x[5] <= 339.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        if (x[7] <= 766.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[6] <= 539.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[0] <= 0.5) {
                                        if (x[6] <= 553.0) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        if (x[4] <= 155.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                else {
                    if (x[4] <= 159.0) {
                        votes[3] += 1;
                    }

                    else {
                        if (x[1] <= 2.5) {
                            if (x[4] <= 180.5) {
                                if (x[2] <= 22.5) {
                                    if (x[4] <= 162.5) {
                                        if (x[0] <= 1.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        if (x[7] <= 788.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                if (x[2] <= 26.5) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }

                        else {
                            if (x[4] <= 216.5) {
                                if (x[1] <= 3.5) {
                                    if (x[7] <= 792.5) {
                                        if (x[4] <= 174.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        if (x[3] <= 80.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[3] <= 80.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        if (x[4] <= 212.0) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[5] <= 427.0) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }
                    }
                }
            }

            else {
                if (x[3] <= 109.5) {
                    if (x[5] <= 426.0) {
                        if (x[2] <= 38.0) {
                            votes[0] += 1;
                        }

                        else {
                            votes[1] += 1;
                        }
                    }

                    else {
                        if (x[7] <= 854.5) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[5] <= 428.5) {
                                votes[3] += 1;
                            }

                            else {
                                if (x[1] <= 5.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[3] <= 108.0) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }
                        }
                    }
                }

                else {
                    if (x[2] <= 40.5) {
                        if (x[5] <= 441.5) {
                            if (x[4] <= 248.5) {
                                if (x[6] <= 634.5) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        else {
                            if (x[3] <= 112.5) {
                                votes[0] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }
                    }

                    else {
                        if (x[1] <= 5.5) {
                            votes[0] += 1;
                        }

                        else {
                            votes[1] += 1;
                        }
                    }
                }
            }
        }

        else {
            if (x[1] <= 5.5) {
                if (x[5] <= 541.0) {
                    if (x[4] <= 318.5) {
                        if (x[3] <= 123.0) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[4] <= 297.0) {
                                if (x[2] <= 40.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }
                    }

                    else {
                        votes[3] += 1;
                    }
                }

                else {
                    if (x[1] <= 4.5) {
                        if (x[4] <= 347.5) {
                            if (x[4] <= 341.5) {
                                if (x[2] <= 43.5) {
                                    if (x[3] <= 109.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        if (x[0] <= 3.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                if (x[3] <= 133.5) {
                                    if (x[3] <= 106.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        if (x[5] <= 582.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }

                        else {
                            if (x[7] <= 1102.5) {
                                if (x[5] <= 605.5) {
                                    if (x[2] <= 44.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        if (x[7] <= 1071.0) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[6] <= 855.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[7] <= 1095.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }
                    }

                    else {
                        if (x[0] <= 5.5) {
                            if (x[3] <= 136.0) {
                                if (x[3] <= 122.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[6] <= 851.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        if (x[2] <= 37.5) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        else {
                            if (x[4] <= 360.5) {
                                if (x[7] <= 1090.5) {
                                    if (x[3] <= 137.5) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        if (x[2] <= 36.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                if (x[5] <= 621.0) {
                                    if (x[5] <= 617.5) {
                                        if (x[5] <= 590.5) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    votes[2] += 1;
                                }
                            }
                        }
                    }
                }
            }

            else {
                if (x[4] <= 331.5) {
                    if (x[6] <= 747.5) {
                        if (x[2] <= 40.0) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[3] <= 130.0) {
                                if (x[7] <= 905.5) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                votes[3] += 1;
                            }
                        }
                    }

                    else {
                        if (x[5] <= 539.0) {
                            if (x[7] <= 978.0) {
                                votes[0] += 1;
                            }

                            else {
                                votes[2] += 1;
                            }
                        }

                        else {
                            if (x[5] <= 545.5) {
                                votes[3] += 1;
                            }

                            else {
                                votes[0] += 1;
                            }
                        }
                    }
                }

                else {
                    if (x[3] <= 161.5) {
                        if (x[6] <= 860.0) {
                            if (x[2] <= 42.5) {
                                if (x[7] <= 978.5) {
                                    if (x[7] <= 971.5) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    if (x[4] <= 358.5) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        if (x[7] <= 1023.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[3] <= 159.5) {
                                    if (x[4] <= 350.0) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        if (x[4] <= 365.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[5] <= 614.0) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            votes[0] += 1;
                        }
                    }

                    else {
                        if (x[7] <= 1071.0) {
                            votes[3] += 1;
                        }

                        else {
                            if (x[3] <= 168.5) {
                                if (x[5] <= 625.0) {
                                    votes[2] += 1;
                                }

                                else {
                                    if (x[6] <= 858.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        votes[2] += 1;
                                    }
                                }
                            }

                            else {
                                votes[3] += 1;
                            }
                        }
                    }
                }
            }
        }
    }

    else {
        votes[4] += 1;
    }

    // tree #2
    if (x[3] <= 325.0) {
        if (x[7] <= 877.5) {
            if (x[5] <= 425.5) {
                if (x[7] <= 755.5) {
                    if (x[5] <= 334.0) {
                        votes[1] += 1;
                    }

                    else {
                        if (x[0] <= 0.5) {
                            votes[0] += 1;
                        }

                        else {
                            votes[1] += 1;
                        }
                    }
                }

                else {
                    if (x[3] <= 42.5) {
                        if (x[5] <= 338.5) {
                            if (x[3] <= 37.0) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[2] <= 10.5) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        else {
                            if (x[2] <= 9.5) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[6] <= 551.5) {
                                    if (x[1] <= 0.5) {
                                        if (x[2] <= 10.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    if (x[2] <= 13.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }
                        }
                    }

                    else {
                        if (x[5] <= 414.5) {
                            if (x[7] <= 814.5) {
                                if (x[6] <= 551.5) {
                                    if (x[6] <= 539.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[7] <= 766.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[4] <= 156.5) {
                                        if (x[5] <= 343.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }

                                    else {
                                        if (x[6] <= 554.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[4] <= 227.0) {
                                    if (x[2] <= 27.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[5] <= 409.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[2] <= 34.0) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            votes[1] += 1;
                        }
                    }
                }
            }

            else {
                if (x[4] <= 248.0) {
                    if (x[7] <= 855.0) {
                        if (x[6] <= 632.5) {
                            if (x[5] <= 431.0) {
                                votes[3] += 1;
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        else {
                            if (x[4] <= 246.0) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[2] <= 39.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }
                    }

                    else {
                        if (x[4] <= 243.5) {
                            if (x[6] <= 644.0) {
                                votes[0] += 1;
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        else {
                            votes[3] += 1;
                        }
                    }
                }

                else {
                    if (x[1] <= 5.5) {
                        votes[0] += 1;
                    }

                    else {
                        if (x[5] <= 459.0) {
                            if (x[5] <= 451.5) {
                                if (x[3] <= 110.5) {
                                    if (x[7] <= 864.0) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                if (x[2] <= 40.0) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        else {
                            votes[3] += 1;
                        }
                    }
                }
            }
        }

        else {
            if (x[1] <= 5.5) {
                if (x[5] <= 541.0) {
                    votes[0] += 1;
                }

                else {
                    if (x[4] <= 329.0) {
                        if (x[2] <= 43.5) {
                            if (x[3] <= 126.0) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[7] <= 1007.0) {
                                    votes[3] += 1;
                                }

                                else {
                                    if (x[5] <= 555.0) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            votes[3] += 1;
                        }
                    }

                    else {
                        if (x[0] <= 5.5) {
                            if (x[6] <= 854.5) {
                                if (x[6] <= 851.5) {
                                    if (x[1] <= 4.5) {
                                        if (x[1] <= 2.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        if (x[4] <= 357.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[5] <= 603.0) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[5] <= 606.5) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[7] <= 1102.5) {
                                    if (x[5] <= 607.5) {
                                        if (x[6] <= 860.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        if (x[7] <= 1089.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }

                        else {
                            if (x[1] <= 4.5) {
                                if (x[3] <= 129.5) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                if (x[3] <= 151.5) {
                                    if (x[7] <= 1089.5) {
                                        if (x[4] <= 371.0) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    if (x[5] <= 621.0) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[2] += 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            else {
                if (x[4] <= 332.0) {
                    if (x[5] <= 538.0) {
                        if (x[7] <= 882.0) {
                            votes[3] += 1;
                        }

                        else {
                            if (x[2] <= 42.5) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[5] <= 502.5) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }
                    }

                    else {
                        votes[3] += 1;
                    }
                }

                else {
                    if (x[2] <= 43.5) {
                        if (x[3] <= 163.5) {
                            if (x[6] <= 860.0) {
                                if (x[6] <= 821.0) {
                                    if (x[4] <= 365.5) {
                                        if (x[3] <= 145.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }

                                    else {
                                        if (x[2] <= 42.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[3] <= 162.5) {
                                        if (x[7] <= 1088.5) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[4] <= 392.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[2] += 1;
                                }
                            }
                        }

                        else {
                            if (x[0] <= 5.5) {
                                votes[0] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }
                    }

                    else {
                        if (x[5] <= 614.5) {
                            if (x[4] <= 362.5) {
                                votes[2] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        else {
                            votes[2] += 1;
                        }
                    }
                }
            }
        }
    }

    else {
        votes[4] += 1;
    }

    // tree #3
    if (x[5] <= 745.5) {
        if (x[7] <= 866.5) {
            if (x[7] <= 788.5) {
                if (x[7] <= 752.5) {
                    votes[1] += 1;
                }

                else {
                    if (x[6] <= 539.5) {
                        votes[0] += 1;
                    }

                    else {
                        if (x[3] <= 64.0) {
                            if (x[6] <= 560.5) {
                                if (x[2] <= 15.5) {
                                    if (x[6] <= 551.5) {
                                        if (x[7] <= 766.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        if (x[4] <= 156.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[5] <= 354.5) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        if (x[3] <= 54.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[2] <= 18.5) {
                                    if (x[0] <= 0.5) {
                                        if (x[4] <= 169.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    if (x[6] <= 574.0) {
                                        if (x[6] <= 566.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            votes[1] += 1;
                        }
                    }
                }
            }

            else {
                if (x[3] <= 73.5) {
                    if (x[3] <= 66.5) {
                        if (x[4] <= 178.5) {
                            votes[3] += 1;
                        }

                        else {
                            votes[0] += 1;
                        }
                    }

                    else {
                        if (x[4] <= 188.0) {
                            if (x[6] <= 584.0) {
                                votes[3] += 1;
                            }

                            else {
                                if (x[6] <= 586.5) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }

                        else {
                            votes[0] += 1;
                        }
                    }
                }

                else {
                    if (x[6] <= 632.0) {
                        if (x[1] <= 4.5) {
                            if (x[6] <= 597.5) {
                                if (x[7] <= 807.5) {
                                    if (x[3] <= 76.5) {
                                        if (x[1] <= 2.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                if (x[7] <= 827.0) {
                                    if (x[5] <= 392.0) {
                                        if (x[3] <= 82.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        if (x[7] <= 819.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }

                        else {
                            if (x[0] <= 4.5) {
                                votes[3] += 1;
                            }

                            else {
                                if (x[3] <= 84.0) {
                                    votes[3] += 1;
                                }

                                else {
                                    if (x[4] <= 227.5) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        if (x[4] <= 228.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    else {
                        if (x[3] <= 108.5) {
                            if (x[6] <= 634.5) {
                                if (x[5] <= 428.0) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        else {
                            if (x[1] <= 5.5) {
                                if (x[6] <= 651.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                if (x[4] <= 266.5) {
                                    if (x[6] <= 639.5) {
                                        if (x[7] <= 848.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        if (x[5] <= 441.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }
                    }
                }
            }
        }

        else {
            if (x[1] <= 5.5) {
                if (x[7] <= 1001.5) {
                    if (x[3] <= 138.5) {
                        votes[0] += 1;
                    }

                    else {
                        if (x[6] <= 751.5) {
                            votes[3] += 1;
                        }

                        else {
                            votes[0] += 1;
                        }
                    }
                }

                else {
                    if (x[1] <= 4.5) {
                        if (x[7] <= 1058.5) {
                            if (x[2] <= 42.5) {
                                if (x[5] <= 583.0) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                if (x[2] <= 45.5) {
                                    if (x[4] <= 311.0) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[3] <= 122.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }

                        else {
                            if (x[4] <= 337.5) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[6] <= 858.5) {
                                    if (x[5] <= 600.5) {
                                        if (x[2] <= 20.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    if (x[5] <= 610.5) {
                                        if (x[0] <= 4.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }
                        }
                    }

                    else {
                        if (x[2] <= 33.5) {
                            if (x[6] <= 848.5) {
                                if (x[2] <= 29.5) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                if (x[7] <= 1089.5) {
                                    votes[2] += 1;
                                }

                                else {
                                    if (x[4] <= 357.0) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        if (x[4] <= 363.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }
                                }
                            }
                        }

                        else {
                            if (x[3] <= 134.5) {
                                if (x[4] <= 365.5) {
                                    if (x[6] <= 789.0) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                if (x[7] <= 1044.5) {
                                    if (x[4] <= 370.5) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    if (x[5] <= 624.5) {
                                        if (x[5] <= 617.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        votes[2] += 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            else {
                if (x[5] <= 537.5) {
                    if (x[4] <= 318.0) {
                        if (x[7] <= 881.5) {
                            if (x[7] <= 868.5) {
                                votes[0] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        else {
                            if (x[2] <= 42.5) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[5] <= 502.5) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }
                    }

                    else {
                        if (x[7] <= 969.5) {
                            votes[3] += 1;
                        }

                        else {
                            votes[0] += 1;
                        }
                    }
                }

                else {
                    if (x[3] <= 163.5) {
                        if (x[2] <= 43.5) {
                            if (x[3] <= 132.0) {
                                votes[2] += 1;
                            }

                            else {
                                if (x[3] <= 135.0) {
                                    if (x[5] <= 607.5) {
                                        if (x[0] <= 5.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    if (x[6] <= 859.5) {
                                        if (x[4] <= 335.5) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }

                                    else {
                                        if (x[7] <= 1085.5) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }
                            }
                        }

                        else {
                            if (x[7] <= 1051.0) {
                                votes[3] += 1;
                            }

                            else {
                                if (x[7] <= 1070.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[2] += 1;
                                }
                            }
                        }
                    }

                    else {
                        if (x[2] <= 41.5) {
                            votes[2] += 1;
                        }

                        else {
                            if (x[4] <= 393.5) {
                                votes[3] += 1;
                            }

                            else {
                                votes[2] += 1;
                            }
                        }
                    }
                }
            }
        }
    }

    else {
        votes[4] += 1;
    }

    // tree #4
    if (x[0] <= 72.0) {
        if (x[4] <= 266.5) {
            if (x[5] <= 425.5) {
                if (x[6] <= 551.5) {
                    if (x[3] <= 41.5) {
                        if (x[7] <= 752.0) {
                            votes[1] += 1;
                        }

                        else {
                            if (x[4] <= 148.0) {
                                if (x[2] <= 9.5) {
                                    if (x[7] <= 758.0) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                if (x[0] <= 0.5) {
                                    votes[1] += 1;
                                }

                                else {
                                    if (x[5] <= 338.0) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }
                        }
                    }

                    else {
                        if (x[3] <= 43.5) {
                            if (x[7] <= 757.5) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[7] <= 760.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        else {
                            votes[1] += 1;
                        }
                    }
                }

                else {
                    if (x[4] <= 156.5) {
                        if (x[2] <= 12.5) {
                            if (x[5] <= 342.5) {
                                votes[0] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        else {
                            if (x[3] <= 44.0) {
                                if (x[0] <= 1.5) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[1] += 1;
                            }
                        }
                    }

                    else {
                        if (x[2] <= 35.5) {
                            if (x[7] <= 818.5) {
                                if (x[6] <= 557.5) {
                                    if (x[3] <= 42.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[4] <= 157.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[3] <= 73.5) {
                                        if (x[0] <= 3.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        if (x[6] <= 585.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[5] <= 414.5) {
                                    if (x[2] <= 29.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[4] <= 227.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        else {
                            votes[1] += 1;
                        }
                    }
                }
            }

            else {
                if (x[3] <= 110.5) {
                    if (x[6] <= 634.5) {
                        if (x[4] <= 243.5) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[0] <= 5.5) {
                                votes[0] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }
                    }

                    else {
                        if (x[5] <= 447.0) {
                            if (x[7] <= 857.5) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[7] <= 858.5) {
                                    votes[1] += 1;
                                }

                                else {
                                    if (x[7] <= 863.5) {
                                        if (x[6] <= 647.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            votes[1] += 1;
                        }
                    }
                }

                else {
                    if (x[2] <= 39.5) {
                        if (x[1] <= 5.5) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[5] <= 434.0) {
                                votes[1] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }
                    }

                    else {
                        if (x[0] <= 5.5) {
                            votes[3] += 1;
                        }

                        else {
                            if (x[1] <= 5.5) {
                                votes[3] += 1;
                            }

                            else {
                                votes[1] += 1;
                            }
                        }
                    }
                }
            }
        }

        else {
            if (x[1] <= 5.5) {
                if (x[4] <= 316.0) {
                    if (x[7] <= 898.0) {
                        if (x[6] <= 686.0) {
                            votes[0] += 1;
                        }

                        else {
                            votes[3] += 1;
                        }
                    }

                    else {
                        votes[0] += 1;
                    }
                }

                else {
                    if (x[2] <= 42.5) {
                        if (x[7] <= 1088.5) {
                            if (x[0] <= 5.5) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[7] <= 971.5) {
                                    votes[3] += 1;
                                }

                                else {
                                    if (x[4] <= 359.5) {
                                        if (x[3] <= 122.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }

                                    else {
                                        if (x[5] <= 621.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }
                                }
                            }
                        }

                        else {
                            if (x[6] <= 860.5) {
                                if (x[3] <= 134.0) {
                                    if (x[4] <= 354.0) {
                                        if (x[3] <= 118.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        if (x[7] <= 1090.5) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }
                    }

                    else {
                        if (x[4] <= 343.5) {
                            if (x[3] <= 139.0) {
                                if (x[6] <= 826.5) {
                                    votes[3] += 1;
                                }

                                else {
                                    if (x[5] <= 580.0) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[3] <= 125.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        else {
                            if (x[6] <= 823.5) {
                                if (x[2] <= 44.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                if (x[4] <= 367.5) {
                                    if (x[7] <= 1087.0) {
                                        if (x[7] <= 1056.0) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        if (x[4] <= 358.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }

                                else {
                                    votes[2] += 1;
                                }
                            }
                        }
                    }
                }
            }

            else {
                if (x[7] <= 967.0) {
                    if (x[2] <= 39.5) {
                        votes[0] += 1;
                    }

                    else {
                        if (x[6] <= 708.5) {
                            votes[3] += 1;
                        }

                        else {
                            if (x[5] <= 515.5) {
                                votes[0] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }
                    }
                }

                else {
                    if (x[3] <= 163.5) {
                        if (x[6] <= 860.5) {
                            if (x[7] <= 1072.5) {
                                if (x[2] <= 43.5) {
                                    if (x[5] <= 616.5) {
                                        if (x[4] <= 317.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    if (x[3] <= 154.5) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[6] <= 852.5) {
                                    votes[2] += 1;
                                }

                                else {
                                    if (x[2] <= 33.5) {
                                        if (x[4] <= 360.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }

                                    else {
                                        if (x[7] <= 1089.0) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }
                                }
                            }
                        }

                        else {
                            votes[0] += 1;
                        }
                    }

                    else {
                        if (x[2] <= 41.5) {
                            if (x[4] <= 386.5) {
                                votes[2] += 1;
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        else {
                            if (x[4] <= 393.5) {
                                votes[3] += 1;
                            }

                            else {
                                votes[2] += 1;
                            }
                        }
                    }
                }
            }
        }
    }

    else {
        votes[4] += 1;
    }

    // tree #5
    if (x[7] <= 1174.5) {
        if (x[7] <= 870.5) {
            if (x[7] <= 840.0) {
                if (x[6] <= 555.5) {
                    if (x[2] <= 10.5) {
                        if (x[7] <= 752.0) {
                            votes[1] += 1;
                        }

                        else {
                            if (x[7] <= 757.5) {
                                if (x[5] <= 331.5) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                if (x[5] <= 339.5) {
                                    if (x[3] <= 39.5) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }
                    }

                    else {
                        if (x[3] <= 43.5) {
                            if (x[7] <= 764.5) {
                                if (x[5] <= 338.0) {
                                    votes[1] += 1;
                                }

                                else {
                                    if (x[6] <= 548.0) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[7] <= 766.5) {
                                    if (x[2] <= 11.5) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    if (x[4] <= 155.5) {
                                        if (x[4] <= 153.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            if (x[6] <= 554.5) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[4] <= 156.5) {
                                    votes[3] += 1;
                                }

                                else {
                                    if (x[5] <= 346.5) {
                                        if (x[7] <= 769.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }
                        }
                    }
                }

                else {
                    if (x[4] <= 160.5) {
                        if (x[4] <= 155.5) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[2] <= 16.0) {
                                votes[3] += 1;
                            }

                            else {
                                if (x[0] <= 2.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[2] <= 19.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }
                        }
                    }

                    else {
                        if (x[6] <= 628.5) {
                            if (x[5] <= 354.5) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[3] <= 64.0) {
                                    if (x[0] <= 1.5) {
                                        if (x[5] <= 362.0) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        if (x[3] <= 60.0) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[7] <= 788.5) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        if (x[3] <= 73.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }
                            }
                        }

                        else {
                            votes[3] += 1;
                        }
                    }
                }
            }

            else {
                if (x[4] <= 246.0) {
                    if (x[5] <= 425.5) {
                        if (x[4] <= 230.5) {
                            votes[0] += 1;
                        }

                        else {
                            votes[1] += 1;
                        }
                    }

                    else {
                        if (x[6] <= 647.5) {
                            if (x[6] <= 632.5) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[5] <= 428.0) {
                                    if (x[6] <= 636.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }

                        else {
                            votes[1] += 1;
                        }
                    }
                }

                else {
                    if (x[0] <= 5.5) {
                        if (x[7] <= 864.0) {
                            votes[0] += 1;
                        }

                        else {
                            votes[3] += 1;
                        }
                    }

                    else {
                        if (x[1] <= 5.5) {
                            if (x[3] <= 109.5) {
                                votes[0] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        else {
                            if (x[4] <= 266.5) {
                                if (x[4] <= 259.5) {
                                    if (x[6] <= 645.5) {
                                        if (x[5] <= 439.0) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }

                                else {
                                    if (x[6] <= 654.0) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }

                            else {
                                votes[3] += 1;
                            }
                        }
                    }
                }
            }
        }

        else {
            if (x[1] <= 4.5) {
                if (x[3] <= 122.5) {
                    if (x[4] <= 340.5) {
                        if (x[5] <= 594.5) {
                            if (x[4] <= 339.5) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[6] <= 850.0) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }

                        else {
                            if (x[6] <= 848.0) {
                                votes[3] += 1;
                            }

                            else {
                                if (x[2] <= 19.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }
                    }

                    else {
                        if (x[3] <= 118.5) {
                            if (x[0] <= 1.5) {
                                if (x[2] <= 23.5) {
                                    if (x[3] <= 107.0) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                if (x[6] <= 854.5) {
                                    if (x[6] <= 851.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    if (x[3] <= 117.5) {
                                        if (x[2] <= 31.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        if (x[2] <= 32.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }
                            }
                        }

                        else {
                            votes[0] += 1;
                        }
                    }
                }

                else {
                    if (x[2] <= 40.5) {
                        votes[3] += 1;
                    }

                    else {
                        if (x[3] <= 125.5) {
                            if (x[3] <= 124.5) {
                                votes[3] += 1;
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        else {
                            if (x[3] <= 127.5) {
                                votes[3] += 1;
                            }

                            else {
                                if (x[5] <= 580.0) {
                                    if (x[7] <= 1053.0) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    if (x[4] <= 343.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        if (x[3] <= 130.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            else {
                if (x[4] <= 317.5) {
                    if (x[4] <= 259.0) {
                        if (x[4] <= 254.5) {
                            votes[1] += 1;
                        }

                        else {
                            votes[3] += 1;
                        }
                    }

                    else {
                        if (x[7] <= 1016.0) {
                            if (x[7] <= 928.5) {
                                if (x[3] <= 123.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[2] <= 39.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        else {
                            votes[3] += 1;
                        }
                    }
                }

                else {
                    if (x[4] <= 326.5) {
                        if (x[5] <= 545.5) {
                            if (x[6] <= 756.5) {
                                if (x[4] <= 322.5) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        else {
                            if (x[7] <= 991.0) {
                                votes[2] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }
                    }

                    else {
                        if (x[7] <= 1089.5) {
                            if (x[2] <= 43.5) {
                                if (x[6] <= 859.0) {
                                    if (x[3] <= 163.5) {
                                        if (x[1] <= 5.5) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }

                                    else {
                                        if (x[2] <= 41.5) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[4] <= 392.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[2] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[5] <= 588.0) {
                                    votes[3] += 1;
                                }

                                else {
                                    if (x[2] <= 44.5) {
                                        if (x[0] <= 5.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            if (x[6] <= 849.5) {
                                votes[3] += 1;
                            }

                            else {
                                if (x[4] <= 388.5) {
                                    if (x[6] <= 852.0) {
                                        if (x[1] <= 5.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    votes[2] += 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    else {
        votes[4] += 1;
    }

    // tree #6
    if (x[3] <= 325.0) {
        if (x[4] <= 259.5) {
            if (x[7] <= 777.5) {
                if (x[3] <= 42.5) {
                    if (x[5] <= 331.5) {
                        votes[1] += 1;
                    }

                    else {
                        if (x[1] <= 0.5) {
                            if (x[4] <= 147.5) {
                                if (x[6] <= 540.0) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[7] <= 755.0) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[2] <= 8.0) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[5] <= 343.0) {
                                    if (x[4] <= 151.5) {
                                        if (x[6] <= 543.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }

                        else {
                            votes[0] += 1;
                        }
                    }
                }

                else {
                    if (x[2] <= 15.5) {
                        if (x[3] <= 46.5) {
                            if (x[3] <= 43.5) {
                                if (x[7] <= 765.5) {
                                    if (x[6] <= 548.0) {
                                        if (x[2] <= 11.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }

                                else {
                                    if (x[1] <= 0.5) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[2] <= 13.5) {
                                    votes[1] += 1;
                                }

                                else {
                                    if (x[4] <= 155.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            if (x[6] <= 554.5) {
                                if (x[2] <= 11.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[7] <= 768.5) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }

                            else {
                                votes[3] += 1;
                            }
                        }
                    }

                    else {
                        if (x[6] <= 560.5) {
                            if (x[4] <= 157.5) {
                                if (x[6] <= 553.5) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        else {
                            if (x[3] <= 51.0) {
                                votes[0] += 1;
                            }

                            else {
                                votes[1] += 1;
                            }
                        }
                    }
                }
            }

            else {
                if (x[7] <= 841.0) {
                    if (x[3] <= 83.0) {
                        if (x[0] <= 2.5) {
                            if (x[1] <= 1.5) {
                                if (x[4] <= 170.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[2] <= 20.5) {
                                        if (x[4] <= 177.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        if (x[4] <= 181.0) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        else {
                            if (x[7] <= 785.5) {
                                votes[3] += 1;
                            }

                            else {
                                if (x[6] <= 597.5) {
                                    if (x[7] <= 788.0) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        if (x[3] <= 79.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[3] <= 79.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }
                        }
                    }

                    else {
                        if (x[3] <= 103.0) {
                            if (x[6] <= 630.5) {
                                if (x[1] <= 4.5) {
                                    if (x[6] <= 608.0) {
                                        if (x[6] <= 596.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        else {
                            votes[3] += 1;
                        }
                    }
                }

                else {
                    if (x[3] <= 108.5) {
                        if (x[6] <= 632.0) {
                            if (x[2] <= 36.0) {
                                votes[0] += 1;
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        else {
                            if (x[6] <= 634.5) {
                                if (x[7] <= 854.0) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }
                    }

                    else {
                        if (x[1] <= 5.5) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[4] <= 242.0) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[6] <= 639.5) {
                                    if (x[5] <= 434.0) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    if (x[4] <= 254.5) {
                                        if (x[7] <= 856.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        if (x[7] <= 864.0) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        else {
            if (x[1] <= 4.5) {
                if (x[3] <= 125.5) {
                    if (x[5] <= 597.5) {
                        if (x[3] <= 110.5) {
                            if (x[4] <= 341.5) {
                                if (x[6] <= 848.5) {
                                    if (x[6] <= 846.5) {
                                        if (x[4] <= 340.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        else {
                            if (x[2] <= 43.5) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[6] <= 781.5) {
                                    if (x[6] <= 742.0) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }
                    }

                    else {
                        if (x[3] <= 117.5) {
                            if (x[2] <= 23.5) {
                                if (x[1] <= 0.5) {
                                    if (x[2] <= 21.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        else {
                            if (x[3] <= 121.5) {
                                if (x[2] <= 28.5) {
                                    votes[3] += 1;
                                }

                                else {
                                    if (x[2] <= 34.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[7] <= 1100.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                votes[3] += 1;
                            }
                        }
                    }
                }

                else {
                    if (x[6] <= 859.5) {
                        if (x[7] <= 1101.5) {
                            if (x[6] <= 824.5) {
                                if (x[3] <= 130.5) {
                                    if (x[6] <= 821.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        if (x[4] <= 348.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[4] <= 346.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[4] <= 352.0) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[0] <= 5.5) {
                                    if (x[7] <= 1080.0) {
                                        if (x[6] <= 843.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    if (x[6] <= 844.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            votes[0] += 1;
                        }
                    }

                    else {
                        if (x[4] <= 367.0) {
                            votes[0] += 1;
                        }

                        else {
                            votes[3] += 1;
                        }
                    }
                }
            }

            else {
                if (x[4] <= 317.5) {
                    if (x[4] <= 295.0) {
                        if (x[2] <= 40.5) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[3] <= 115.0) {
                                votes[0] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }
                    }

                    else {
                        if (x[6] <= 789.0) {
                            votes[0] += 1;
                        }

                        else {
                            votes[3] += 1;
                        }
                    }
                }

                else {
                    if (x[2] <= 43.5) {
                        if (x[0] <= 5.5) {
                            if (x[3] <= 153.0) {
                                if (x[1] <= 5.5) {
                                    if (x[5] <= 597.0) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        if (x[7] <= 1090.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[6] <= 754.0) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        if (x[2] <= 32.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[3] <= 159.0) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[2] <= 41.5) {
                                        if (x[7] <= 1079.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }

                                    else {
                                        if (x[7] <= 1074.0) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }
                            }
                        }

                        else {
                            if (x[3] <= 163.5) {
                                if (x[7] <= 1091.0) {
                                    if (x[3] <= 151.5) {
                                        if (x[4] <= 332.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }

                                    else {
                                        if (x[3] <= 160.5) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                if (x[2] <= 41.5) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }
                    }

                    else {
                        if (x[7] <= 1063.0) {
                            if (x[5] <= 589.0) {
                                votes[3] += 1;
                            }

                            else {
                                if (x[4] <= 363.5) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }

                        else {
                            if (x[7] <= 1074.0) {
                                votes[0] += 1;
                            }

                            else {
                                votes[2] += 1;
                            }
                        }
                    }
                }
            }
        }
    }

    else {
        votes[4] += 1;
    }

    // tree #7
    if (x[3] <= 324.5) {
        if (x[7] <= 866.5) {
            if (x[0] <= 4.5) {
                if (x[7] <= 756.5) {
                    if (x[3] <= 37.5) {
                        if (x[4] <= 142.5) {
                            votes[1] += 1;
                        }

                        else {
                            votes[0] += 1;
                        }
                    }

                    else {
                        if (x[5] <= 334.5) {
                            votes[1] += 1;
                        }

                        else {
                            if (x[4] <= 146.0) {
                                votes[0] += 1;
                            }

                            else {
                                votes[1] += 1;
                            }
                        }
                    }
                }

                else {
                    if (x[3] <= 41.5) {
                        if (x[7] <= 758.5) {
                            if (x[4] <= 150.0) {
                                if (x[2] <= 8.0) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        else {
                            if (x[1] <= 0.5) {
                                if (x[7] <= 765.5) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }
                    }

                    else {
                        if (x[6] <= 554.5) {
                            if (x[4] <= 155.5) {
                                if (x[6] <= 551.5) {
                                    if (x[4] <= 151.0) {
                                        if (x[0] <= 1.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        if (x[7] <= 766.0) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[2] <= 15.0) {
                                        if (x[7] <= 767.0) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[4] <= 165.5) {
                                    votes[1] += 1;
                                }

                                else {
                                    if (x[6] <= 553.0) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            if (x[2] <= 16.5) {
                                if (x[3] <= 45.5) {
                                    if (x[4] <= 157.0) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }

                                else {
                                    if (x[5] <= 360.5) {
                                        if (x[4] <= 161.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[2] <= 34.0) {
                                    if (x[7] <= 780.5) {
                                        if (x[4] <= 162.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        if (x[3] <= 67.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }
                    }
                }
            }

            else {
                if (x[3] <= 110.5) {
                    if (x[5] <= 425.5) {
                        if (x[2] <= 31.0) {
                            if (x[7] <= 815.5) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[4] <= 198.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }

                        else {
                            if (x[1] <= 5.5) {
                                if (x[7] <= 843.5) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                if (x[2] <= 38.0) {
                                    if (x[2] <= 36.0) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }
                    }

                    else {
                        if (x[6] <= 634.5) {
                            if (x[2] <= 38.5) {
                                votes[0] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        else {
                            if (x[2] <= 41.5) {
                                if (x[6] <= 647.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[7] <= 863.5) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[4] <= 253.0) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }
                    }
                }

                else {
                    if (x[2] <= 40.5) {
                        if (x[7] <= 846.5) {
                            votes[1] += 1;
                        }

                        else {
                            if (x[7] <= 861.0) {
                                if (x[4] <= 248.0) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[4] <= 251.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[5] <= 440.5) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }
                    }

                    else {
                        if (x[1] <= 5.5) {
                            votes[3] += 1;
                        }

                        else {
                            votes[1] += 1;
                        }
                    }
                }
            }
        }

        else {
            if (x[4] <= 353.5) {
                if (x[4] <= 312.0) {
                    if (x[1] <= 5.5) {
                        if (x[3] <= 123.5) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[6] <= 711.0) {
                                if (x[2] <= 40.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }
                    }

                    else {
                        if (x[2] <= 40.5) {
                            if (x[4] <= 256.0) {
                                votes[1] += 1;
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        else {
                            if (x[6] <= 708.5) {
                                if (x[5] <= 458.0) {
                                    if (x[3] <= 113.0) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }
                    }
                }

                else {
                    if (x[1] <= 5.5) {
                        if (x[2] <= 43.5) {
                            if (x[1] <= 1.5) {
                                if (x[7] <= 1097.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                if (x[6] <= 848.5) {
                                    if (x[6] <= 847.5) {
                                        if (x[3] <= 125.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    if (x[2] <= 34.5) {
                                        if (x[3] <= 114.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        if (x[3] <= 119.0) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }
                            }
                        }

                        else {
                            if (x[3] <= 122.0) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[3] <= 130.5) {
                                    votes[3] += 1;
                                }

                                else {
                                    if (x[7] <= 1058.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }
                        }
                    }

                    else {
                        if (x[4] <= 349.5) {
                            if (x[5] <= 556.0) {
                                if (x[4] <= 327.0) {
                                    if (x[4] <= 317.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[4] <= 319.0) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[3] <= 150.5) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        if (x[6] <= 759.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[5] <= 560.0) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[2] <= 42.5) {
                                        if (x[6] <= 776.5) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            votes[2] += 1;
                        }
                    }
                }
            }

            else {
                if (x[7] <= 1090.5) {
                    if (x[5] <= 603.5) {
                        if (x[3] <= 163.0) {
                            if (x[4] <= 364.5) {
                                if (x[7] <= 1043.5) {
                                    if (x[4] <= 359.5) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        if (x[7] <= 1021.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[3] <= 131.0) {
                                        if (x[6] <= 848.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }

                                    else {
                                        if (x[5] <= 590.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[6] <= 820.5) {
                                    if (x[5] <= 592.5) {
                                        if (x[4] <= 366.5) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    if (x[2] <= 43.5) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            votes[3] += 1;
                        }
                    }

                    else {
                        if (x[5] <= 610.5) {
                            if (x[7] <= 1089.5) {
                                if (x[7] <= 1088.5) {
                                    votes[2] += 1;
                                }

                                else {
                                    if (x[3] <= 133.0) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[5] <= 607.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[2] += 1;
                                }
                            }
                        }

                        else {
                            if (x[7] <= 1085.5) {
                                if (x[4] <= 382.0) {
                                    if (x[4] <= 374.0) {
                                        if (x[5] <= 616.0) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }

                                    else {
                                        if (x[6] <= 845.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }

                                else {
                                    votes[2] += 1;
                                }
                            }

                            else {
                                if (x[3] <= 151.5) {
                                    if (x[4] <= 374.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        votes[2] += 1;
                                    }
                                }

                                else {
                                    if (x[3] <= 159.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[0] <= 5.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                else {
                    if (x[6] <= 859.5) {
                        votes[3] += 1;
                    }

                    else {
                        votes[0] += 1;
                    }
                }
            }
        }
    }

    else {
        votes[4] += 1;
    }

    // tree #8
    if (x[0] <= 72.0) {
        if (x[7] <= 871.0) {
            if (x[7] <= 814.5) {
                if (x[6] <= 551.5) {
                    if (x[2] <= 7.5) {
                        if (x[6] <= 543.5) {
                            if (x[4] <= 145.5) {
                                votes[3] += 1;
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        else {
                            votes[1] += 1;
                        }
                    }

                    else {
                        if (x[7] <= 766.5) {
                            if (x[2] <= 10.5) {
                                if (x[0] <= 0.5) {
                                    if (x[7] <= 752.0) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        if (x[7] <= 755.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        else {
                            if (x[3] <= 43.5) {
                                votes[0] += 1;
                            }

                            else {
                                votes[1] += 1;
                            }
                        }
                    }
                }

                else {
                    if (x[3] <= 42.5) {
                        votes[3] += 1;
                    }

                    else {
                        if (x[3] <= 65.5) {
                            if (x[7] <= 777.5) {
                                if (x[2] <= 16.5) {
                                    if (x[6] <= 554.5) {
                                        if (x[7] <= 764.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        if (x[4] <= 160.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[6] <= 560.5) {
                                        if (x[0] <= 2.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        if (x[4] <= 160.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[4] <= 181.0) {
                                    if (x[6] <= 569.5) {
                                        if (x[6] <= 566.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }

                                    else {
                                        if (x[0] <= 1.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[5] <= 370.5) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            if (x[7] <= 802.5) {
                                if (x[4] <= 196.0) {
                                    votes[1] += 1;
                                }

                                else {
                                    if (x[7] <= 799.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[2] <= 31.0) {
                                    if (x[7] <= 806.0) {
                                        if (x[6] <= 594.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }
                    }
                }
            }

            else {
                if (x[3] <= 110.5) {
                    if (x[6] <= 632.0) {
                        if (x[3] <= 92.5) {
                            if (x[6] <= 618.0) {
                                votes[0] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        else {
                            if (x[0] <= 4.5) {
                                votes[3] += 1;
                            }

                            else {
                                if (x[5] <= 427.0) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }
                    }

                    else {
                        if (x[2] <= 41.5) {
                            if (x[4] <= 262.5) {
                                if (x[6] <= 634.5) {
                                    if (x[7] <= 854.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    if (x[4] <= 248.0) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[4] <= 249.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        else {
                            if (x[7] <= 855.0) {
                                votes[1] += 1;
                            }

                            else {
                                votes[0] += 1;
                            }
                        }
                    }
                }

                else {
                    if (x[2] <= 40.5) {
                        if (x[7] <= 861.5) {
                            if (x[3] <= 112.5) {
                                if (x[6] <= 642.0) {
                                    if (x[6] <= 638.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        else {
                            votes[1] += 1;
                        }
                    }

                    else {
                        if (x[1] <= 5.5) {
                            votes[3] += 1;
                        }

                        else {
                            votes[1] += 1;
                        }
                    }
                }
            }
        }

        else {
            if (x[1] <= 5.5) {
                if (x[3] <= 142.0) {
                    if (x[7] <= 1004.0) {
                        if (x[3] <= 123.5) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[3] <= 129.5) {
                                if (x[7] <= 998.5) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }
                    }

                    else {
                        if (x[2] <= 34.5) {
                            if (x[5] <= 598.5) {
                                if (x[6] <= 846.5) {
                                    if (x[4] <= 340.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    if (x[7] <= 1099.5) {
                                        if (x[3] <= 111.0) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[4] <= 351.5) {
                                    if (x[2] <= 18.0) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    if (x[7] <= 1090.5) {
                                        if (x[3] <= 128.5) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            if (x[7] <= 1095.0) {
                                if (x[6] <= 818.0) {
                                    if (x[3] <= 122.0) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[5] <= 585.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[1] <= 4.5) {
                                        if (x[3] <= 133.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }

                                    else {
                                        if (x[2] <= 40.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[7] <= 1102.0) {
                                    if (x[4] <= 361.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        if (x[3] <= 131.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }
                    }
                }

                else {
                    if (x[6] <= 853.5) {
                        if (x[7] <= 1085.5) {
                            if (x[5] <= 548.5) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[4] <= 363.5) {
                                    votes[2] += 1;
                                }

                                else {
                                    if (x[6] <= 823.0) {
                                        if (x[2] <= 41.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        votes[2] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            votes[3] += 1;
                        }
                    }

                    else {
                        if (x[6] <= 857.5) {
                            votes[0] += 1;
                        }

                        else {
                            votes[2] += 1;
                        }
                    }
                }
            }

            else {
                if (x[5] <= 531.0) {
                    if (x[5] <= 525.5) {
                        if (x[3] <= 112.5) {
                            if (x[5] <= 440.0) {
                                votes[1] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        else {
                            if (x[2] <= 42.5) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[6] <= 728.0) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }
                    }

                    else {
                        votes[3] += 1;
                    }
                }

                else {
                    if (x[2] <= 43.5) {
                        if (x[6] <= 821.0) {
                            if (x[6] <= 816.0) {
                                if (x[4] <= 331.5) {
                                    if (x[4] <= 322.5) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        if (x[3] <= 142.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[2] <= 42.5) {
                                        if (x[3] <= 152.0) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }

                                    else {
                                        if (x[6] <= 805.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[3] <= 151.0) {
                                    votes[2] += 1;
                                }

                                else {
                                    if (x[4] <= 373.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            if (x[5] <= 610.5) {
                                if (x[6] <= 852.5) {
                                    if (x[4] <= 372.5) {
                                        if (x[7] <= 1088.5) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }

                                    else {
                                        if (x[0] <= 5.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[5] <= 608.5) {
                                        if (x[3] <= 133.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }

                                    else {
                                        votes[2] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[4] <= 371.5) {
                                    if (x[7] <= 1083.5) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    if (x[7] <= 1075.0) {
                                        if (x[2] <= 42.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }

                                    else {
                                        if (x[0] <= 5.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    else {
                        if (x[5] <= 602.0) {
                            if (x[3] <= 154.5) {
                                if (x[5] <= 567.5) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[2] += 1;
                                }
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        else {
                            if (x[7] <= 1066.5) {
                                votes[0] += 1;
                            }

                            else {
                                votes[2] += 1;
                            }
                        }
                    }
                }
            }
        }
    }

    else {
        votes[4] += 1;
    }

    // tree #9
    if (x[7] <= 1174.0) {
        if (x[4] <= 264.0) {
            if (x[6] <= 611.0) {
                if (x[6] <= 557.5) {
                    if (x[3] <= 43.5) {
                        if (x[7] <= 752.5) {
                            votes[1] += 1;
                        }

                        else {
                            if (x[4] <= 147.5) {
                                if (x[7] <= 755.0) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[6] <= 543.0) {
                                        if (x[7] <= 758.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[2] <= 11.5) {
                                    if (x[5] <= 340.5) {
                                        if (x[4] <= 149.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }

                                else {
                                    if (x[7] <= 766.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        if (x[5] <= 344.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    else {
                        if (x[6] <= 554.5) {
                            votes[1] += 1;
                        }

                        else {
                            if (x[3] <= 47.5) {
                                if (x[4] <= 156.0) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                votes[1] += 1;
                            }
                        }
                    }
                }

                else {
                    if (x[3] <= 79.5) {
                        if (x[1] <= 3.5) {
                            if (x[2] <= 15.5) {
                                votes[3] += 1;
                            }

                            else {
                                if (x[2] <= 26.5) {
                                    if (x[4] <= 180.5) {
                                        if (x[6] <= 560.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }

                                else {
                                    if (x[1] <= 2.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        if (x[3] <= 77.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }
                            }
                        }

                        else {
                            if (x[7] <= 813.5) {
                                votes[3] += 1;
                            }

                            else {
                                if (x[2] <= 28.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }
                    }

                    else {
                        if (x[2] <= 31.0) {
                            if (x[3] <= 86.5) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[6] <= 599.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }
                        }

                        else {
                            votes[1] += 1;
                        }
                    }
                }
            }

            else {
                if (x[3] <= 109.5) {
                    if (x[5] <= 425.5) {
                        if (x[5] <= 414.5) {
                            if (x[3] <= 98.0) {
                                votes[0] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        else {
                            votes[1] += 1;
                        }
                    }

                    else {
                        if (x[6] <= 635.0) {
                            if (x[2] <= 40.5) {
                                votes[3] += 1;
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        else {
                            if (x[6] <= 650.5) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[5] <= 447.5) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }
                    }
                }

                else {
                    if (x[1] <= 5.5) {
                        votes[0] += 1;
                    }

                    else {
                        if (x[0] <= 5.5) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[5] <= 451.5) {
                                if (x[4] <= 241.0) {
                                    if (x[4] <= 237.0) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    if (x[2] <= 39.5) {
                                        if (x[7] <= 851.0) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }

                                    else {
                                        if (x[3] <= 110.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }
                    }
                }
            }
        }

        else {
            if (x[1] <= 5.5) {
                if (x[7] <= 1001.5) {
                    if (x[3] <= 146.5) {
                        if (x[3] <= 139.5) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[2] <= 42.0) {
                                votes[3] += 1;
                            }

                            else {
                                votes[0] += 1;
                            }
                        }
                    }

                    else {
                        votes[2] += 1;
                    }
                }

                else {
                    if (x[3] <= 138.5) {
                        if (x[2] <= 33.5) {
                            if (x[0] <= 5.5) {
                                if (x[5] <= 604.5) {
                                    if (x[3] <= 109.5) {
                                        if (x[3] <= 107.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }

                                    else {
                                        if (x[7] <= 1089.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[6] <= 860.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        if (x[1] <= 2.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[6] <= 851.5) {
                                    if (x[4] <= 360.5) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }

                        else {
                            if (x[7] <= 1086.0) {
                                if (x[7] <= 1058.5) {
                                    if (x[3] <= 122.0) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[2] <= 42.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[2] <= 43.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        if (x[3] <= 127.0) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[7] <= 1099.5) {
                                    votes[3] += 1;
                                }

                                else {
                                    if (x[6] <= 856.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[4] <= 347.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    else {
                        if (x[0] <= 5.5) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[7] <= 1085.5) {
                                if (x[2] <= 43.5) {
                                    if (x[3] <= 155.5) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        if (x[3] <= 161.0) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                if (x[6] <= 854.0) {
                                    votes[3] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }
                    }
                }
            }

            else {
                if (x[4] <= 328.5) {
                    if (x[7] <= 967.0) {
                        if (x[6] <= 708.5) {
                            votes[3] += 1;
                        }

                        else {
                            if (x[3] <= 136.0) {
                                votes[0] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }
                    }

                    else {
                        if (x[7] <= 979.0) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[7] <= 986.0) {
                                if (x[3] <= 141.0) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }
                    }
                }

                else {
                    if (x[2] <= 42.5) {
                        if (x[5] <= 614.0) {
                            if (x[7] <= 978.5) {
                                if (x[2] <= 40.5) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                if (x[3] <= 159.5) {
                                    if (x[7] <= 1089.5) {
                                        if (x[5] <= 610.5) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }

                                    else {
                                        if (x[3] <= 131.5) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[5] <= 594.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[2] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            if (x[5] <= 617.5) {
                                votes[3] += 1;
                            }

                            else {
                                if (x[2] <= 40.5) {
                                    if (x[6] <= 857.5) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    if (x[3] <= 166.5) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }
                            }
                        }
                    }

                    else {
                        if (x[7] <= 1050.0) {
                            if (x[5] <= 572.0) {
                                votes[2] += 1;
                            }

                            else {
                                if (x[3] <= 151.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[5] <= 587.5) {
                                        if (x[6] <= 807.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }

                                    else {
                                        if (x[0] <= 5.5) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }
                            }
                        }

                        else {
                            if (x[4] <= 362.5) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[7] <= 1087.5) {
                                    if (x[7] <= 1071.0) {
                                        if (x[6] <= 833.5) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        votes[2] += 1;
                                    }
                                }

                                else {
                                    if (x[6] <= 862.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[2] += 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    else {
        votes[4] += 1;
    }

    // tree #10
    if (x[0] <= 72.0) {
        if (x[4] <= 259.5) {
            if (x[7] <= 840.0) {
                if (x[6] <= 557.5) {
                    if (x[2] <= 7.5) {
                        if (x[4] <= 145.5) {
                            votes[3] += 1;
                        }

                        else {
                            votes[0] += 1;
                        }
                    }

                    else {
                        if (x[3] <= 43.5) {
                            if (x[5] <= 334.5) {
                                if (x[3] <= 37.5) {
                                    if (x[4] <= 142.5) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                if (x[4] <= 146.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[4] <= 148.0) {
                                        if (x[0] <= 1.0) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        if (x[6] <= 551.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }
                            }
                        }

                        else {
                            if (x[6] <= 539.5) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[4] <= 165.5) {
                                    if (x[7] <= 768.5) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        if (x[4] <= 156.0) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[2] <= 14.0) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }
                        }
                    }
                }

                else {
                    if (x[2] <= 31.5) {
                        if (x[6] <= 588.5) {
                            if (x[2] <= 16.5) {
                                if (x[3] <= 56.5) {
                                    votes[3] += 1;
                                }

                                else {
                                    if (x[7] <= 775.0) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[3] <= 74.5) {
                                    if (x[7] <= 788.5) {
                                        if (x[0] <= 1.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        if (x[0] <= 2.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[6] <= 585.5) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        if (x[7] <= 797.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }
                            }
                        }

                        else {
                            if (x[2] <= 30.5) {
                                votes[3] += 1;
                            }

                            else {
                                votes[0] += 1;
                            }
                        }
                    }

                    else {
                        if (x[2] <= 38.0) {
                            if (x[6] <= 615.5) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[3] <= 95.0) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        else {
                            votes[3] += 1;
                        }
                    }
                }
            }

            else {
                if (x[3] <= 110.5) {
                    if (x[5] <= 425.5) {
                        if (x[4] <= 230.5) {
                            votes[0] += 1;
                        }

                        else {
                            votes[1] += 1;
                        }
                    }

                    else {
                        if (x[7] <= 857.0) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[7] <= 860.0) {
                                if (x[6] <= 641.0) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }
                    }
                }

                else {
                    if (x[1] <= 5.5) {
                        votes[3] += 1;
                    }

                    else {
                        if (x[7] <= 856.5) {
                            if (x[6] <= 637.5) {
                                votes[1] += 1;
                            }

                            else {
                                if (x[2] <= 40.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        else {
                            if (x[0] <= 5.5) {
                                votes[3] += 1;
                            }

                            else {
                                votes[1] += 1;
                            }
                        }
                    }
                }
            }
        }

        else {
            if (x[1] <= 5.5) {
                if (x[0] <= 5.5) {
                    if (x[4] <= 346.5) {
                        if (x[7] <= 1004.0) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[7] <= 1008.5) {
                                votes[3] += 1;
                            }

                            else {
                                if (x[4] <= 342.5) {
                                    if (x[7] <= 1099.5) {
                                        if (x[4] <= 340.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    if (x[5] <= 578.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        if (x[3] <= 112.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    else {
                        if (x[3] <= 138.5) {
                            if (x[7] <= 1102.5) {
                                if (x[5] <= 601.5) {
                                    if (x[4] <= 349.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        if (x[7] <= 1053.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[6] <= 858.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        if (x[1] <= 1.5) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[3] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        else {
                            votes[0] += 1;
                        }
                    }
                }

                else {
                    if (x[4] <= 341.5) {
                        if (x[4] <= 315.5) {
                            if (x[7] <= 898.0) {
                                if (x[4] <= 286.0) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[3] += 1;
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        else {
                            votes[3] += 1;
                        }
                    }

                    else {
                        if (x[7] <= 1088.5) {
                            if (x[7] <= 1075.5) {
                                if (x[5] <= 590.5) {
                                    votes[2] += 1;
                                }

                                else {
                                    if (x[3] <= 161.0) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[2] += 1;
                                    }
                                }
                            }

                            else {
                                votes[2] += 1;
                            }
                        }

                        else {
                            if (x[3] <= 129.5) {
                                if (x[2] <= 32.5) {
                                    votes[2] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }
                    }
                }
            }

            else {
                if (x[4] <= 328.5) {
                    if (x[2] <= 40.5) {
                        votes[0] += 1;
                    }

                    else {
                        if (x[6] <= 667.5) {
                            if (x[4] <= 267.0) {
                                votes[1] += 1;
                            }

                            else {
                                votes[3] += 1;
                            }
                        }

                        else {
                            if (x[3] <= 136.0) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[6] <= 747.5) {
                                    votes[3] += 1;
                                }

                                else {
                                    if (x[3] <= 139.5) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        if (x[5] <= 532.5) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                else {
                    if (x[2] <= 42.5) {
                        if (x[3] <= 166.5) {
                            if (x[6] <= 860.5) {
                                if (x[6] <= 821.0) {
                                    if (x[7] <= 1041.5) {
                                        if (x[5] <= 553.0) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    if (x[7] <= 1088.5) {
                                        if (x[5] <= 610.5) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }

                                    else {
                                        if (x[3] <= 132.5) {
                                            votes[2] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        else {
                            votes[3] += 1;
                        }
                    }

                    else {
                        if (x[6] <= 820.5) {
                            if (x[3] <= 154.5) {
                                if (x[7] <= 1012.0) {
                                    votes[2] += 1;
                                }

                                else {
                                    if (x[2] <= 43.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[2] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[0] <= 5.5) {
                                    votes[2] += 1;
                                }

                                else {
                                    if (x[5] <= 582.0) {
                                        if (x[7] <= 1019.0) {
                                            votes[3] += 1;
                                        }

                                        else {
                                            votes[2] += 1;
                                        }
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            if (x[3] <= 155.5) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[3] <= 164.0) {
                                    if (x[2] <= 44.5) {
                                        votes[2] += 1;
                                    }

                                    else {
                                        votes[3] += 1;
                                    }
                                }

                                else {
                                    if (x[3] <= 167.5) {
                                        votes[3] += 1;
                                    }

                                    else {
                                        votes[2] += 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    else {
        votes[4] += 1;
    }

    // return argmax of votes
    uint8_t classIdx = 0;
    float maxVotes = votes[0];

    for (uint8_t i = 1; i < 5; i++) {
        if (votes[i] > maxVotes) {
            classIdx = i;
            maxVotes = votes[i];
        }
    }

    return classIdx;
};
/**
* Predict readable class name
*/
const char* predictLabel(float *x) {
    return idxToLabel(predict(x));
};
/**
* Convert class idx to readable name
*/
const char* idxToLabel(uint8_t classIdx) {
    switch (classIdx) {
        case 0:
        return "bombilla";
        case 1:
        return "mate_1_dedo";
        case 2:
        return "mate_1_mano";
        case 3:
        return "mate_2_dedos";
        case 4:
        return "nada";
        default:
        return "Houston we have a problem";
    }
};