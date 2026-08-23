# outcome_predictor

Predicting NBA game outcomes and testing whether the resulting model finds any real edge
against real sportsbook betting lines.

## What's here

- **`data_analysis.ipynb`** — the main analysis. Loads multi-season NBA game, odds, team
  box score, and player box score data; computes de-vigged market-implied win probabilities;
  benchmarks them against actual outcomes (Brier score, log-loss, calibration); engineers
  leakage-safe rolling team-performance and injury-absence features; trains and evaluates a
  logistic regression model (and a gradient boosting comparison) with a time-based train/test
  split; and runs a threshold-based edge-detection backtest with a genuinely held-out final
  season to check whether any apparent edge actually generalizes.
- **`cpp_backtester/`** — a standalone C++ port of the backtesting/edge-detection engine, built
  around the model's output. See its own [README](cpp_backtester/README.md) for details and
  build instructions.

## Data

Sourced from the [nba-stats-dataset](https://www.kaggle.com/datasets/chevronronson/nba-stats-dataset)
Kaggle dataset (games, odds, team/player box scores across multiple seasons). The raw data
itself isn't included in this repo; `data_analysis.ipynb` expects it to be downloaded via
`kagglehub` into a local `kagglehub/` folder alongside the notebook.

## Key finding

The market's closing (and even opening) lines are hard to beat with box-score-derived
features alone. Across a bigger feature set, a non-linear model, an injury-absence proxy, and
both closing- and opening-line targets, no configuration produced a real, holdout-validated
edge -- a well-tested null result rather than an untested assumption.
